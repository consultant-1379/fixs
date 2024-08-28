	/*
 * FIXS_CCH_JTPClient.cpp
 *
 *  Created on: Oct 20, 2011
 *      Author: eiolbel
 */

#include "FIXS_CCH_JTPClient.h"

// FIXS_CCH_JTPClient

const int FIXS_CCH_JTPClient::JOB_REJECTED = -1;
const int FIXS_CCH_JTPClient::JOB_FAILED = -2;
const int FIXS_CCH_JTPClient::JOB_MISMATCH = -3;
const int  FIXS_CCH_JTPClient::JOB_START_REQUEST_REJECTED = 1;

FIXS_CCH_JTPClient::FIXS_CCH_JTPClient (FIXS_CCH_EventHandle aShutDown)
      : m_shutdown(0)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ JTP CONSTRUNCTOR +++ IN"<< std::endl;
	m_shutdown = aShutDown;

	traceObj = new ACS_TRA_trace("FIXS_CCH_JTPClient");

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();
}


FIXS_CCH_JTPClient::~FIXS_CCH_JTPClient()
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ JTP DESTRUNCTOR +++ IN"<< std::endl;
	//Delete trace object
	if (traceObj) {
		delete (traceObj);
		traceObj = NULL;
	}

	FIXS_CCH_logging = 0;
}

 int FIXS_CCH_JTPClient::newJTPClient (unsigned short Len, ACS_JTP_Job &J1, unsigned short U1, unsigned short U2, char *Msg, int cpId)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ NEWJTPCLIENT +++ IN"<< std::endl;
	int requestResult = 0;
	unsigned short R = 0;
	unsigned short RU1 = 0;
	unsigned short RU2 = 0;

	int resState = -1;
	resState = J1.State();
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " resState: " << resState<< std::endl;

	ACS_JTP_Job::JTP_Node pNode;

    if(cpId == 1) pNode.system_id = 1001;
    else if(cpId == 2) pNode.system_id = 1002;

	std::stringstream ss;
	ss << "before jinitreq() U1 = " << U1  <<  " ";
	ss << "U2 = " << U2 << " ";
	ss << "R = " << R << " ";

	{
		char tmpStr[512] = {0};
		std::string temp = ss.str();
		const char *tmp = temp.c_str();
		snprintf(tmpStr, sizeof(tmpStr) - 1, tmp);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
		tmp = NULL;
	}

	if (J1.jinitreq(&pNode, U1, U2, Len, Msg) == false)
	{// This case is for Job Rejected
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " This case is for Job Rejected"<< std::endl;
		J1.jfault(RU1, RU2, R);
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "(jfault) The job did not initiate" << std::endl;
		std::stringstream ss;
		ss << "(jfault) The job did not initiate" << " ";
		ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
		ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
		ss << "R = " << R;

		{
			char tmpStr[512] = {0};
			std::string temp = ss.str();
			const char *tmp = temp.c_str();
			snprintf(tmpStr, sizeof(tmpStr) - 1, tmp);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
			tmp = NULL;
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" jfault =  " << ss.str().c_str() << std::endl;

		if ((R == 30) || (R == 4)) // 30 is 0x001E - 4 is H'0004
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
					" Protocol version Mismatch (0x001E) or backward compatibility "
					" problem (H'0004) with CP dump" << std::endl;
			// Protocol version Mismatch.
			requestResult = JOB_MISMATCH;
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
								" +++++++++++++++++++++++ JOB FAILURE +++++++++++++++++++++++++++" << std::endl;
						// Protocol version Mismatch.
			requestResult = JOB_REJECTED;
		}
	}

	return requestResult;
}

 int FIXS_CCH_JTPClient::waitForConfermation (ACS_JTP_Job &J1, unsigned short U1, unsigned short U2)
{
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ WAITFORCONFERMATION +++ IN"<< std::endl;
	int resultConfermation = 0;
	unsigned short R = 0;
	unsigned short RU1 = 0;
	unsigned short RU2 = 0;

	// jinitconf can not fail if jexinitreq returns true
	J1.jinitconf(RU1, RU2, R);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jinitconf, The data is modified" << std::endl;
	std::stringstream ss;
	ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
	ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
	ss << "R = " << R << " ";
	ss << "jinitconf, The data is modified";
	// Do something with the result if you like

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ++JINITCONF =  " << ss.str().c_str() << std::endl;

	if ( R != 0 || (U2 != RU2))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jinitconf, The data is modified" << std::endl;
		std::stringstream ss;
		ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
		ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
		ss << "R = " << R << " ";
		ss << "jinitconf, The data is modified";

		{
			char tmpStr[512] = {0};
			std::string temp = ss.str();
			const char *tmp = temp.c_str();
			snprintf(tmpStr, sizeof(tmpStr) - 1, tmp);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
			tmp = NULL;
		}
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " ++JINITCONF =  " << ss.str().c_str() << std::endl;

		if ((R == 30) || (R == 4))// 30 is 0x001E - 4 is H'0004 //TODO
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<
				         " Protocol version Mismatch (0x001E) or backward compatibility "
				         " problem (H'0004) with CP dump" << std::endl;
			// Protocol version Mismatch.
			resultConfermation = JOB_MISMATCH;
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " "<< std::endl;
			resultConfermation = JOB_REJECTED;
		}
	}

	return resultConfermation;
}

 bool FIXS_CCH_JTPClient::waitForJTPResult (ACS_JTP_Job &J1, unsigned short Len, unsigned short U1, unsigned short U2, char *Msg, unsigned short &resCode)
{

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " +++ WAITFORJTPRESULT +++ IN"<< std::endl;
	bool resultJtpRes = true;
	char *RMsg = 0;
	unsigned short R, RU1, RU2, RLen;
	unsigned short RU1_temp;

	JTP_HANDLE PollFd;
	JTP_HANDLE fd = J1.getHandle();
	PollFd = fd;
	//DWORD status;

	JTP_HANDLE * waitHandles = new JTP_HANDLE[2];
	waitHandles[0] = PollFd;
	waitHandles[1] = m_shutdown;

	int status = FIXS_CCH_Event::WaitForEvents(2, waitHandles, 60000);


	//status = WaitForMultipleObjects(2, waitHandles, false, 60000);
	const int WAIT_FAILED = -1;
	const int WAIT_TIMEOUT = -2;
	const int WAIT_OBJECT_0 = 0;


	delete [] waitHandles;

	if (status == WAIT_FAILED) // PollFd failed
	{
		J1.jtermreq();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jresultind, failed signal" << std::endl;
		std::stringstream ss;
		ss << "jresultind, failed signal";

		{
			char tmpStr[512] = {0};
			std::string temp = ss.str();
			const char *tmp = temp.c_str();
			snprintf(tmpStr, sizeof(tmpStr) - 1, tmp);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
			tmp = NULL;
		}
		resultJtpRes = false;
	}
	if (status == WAIT_OBJECT_0) // PollFd event
	{
		bool res = J1.jresultind(RU1, RU2, R, RLen, RMsg);

		RU1_temp = RU1;
		RU1_temp = RU1_temp << 8;
		RU1_temp = RU1_temp >> 8;

		std::stringstream ss;
		ss << "jresultind, The data is :";
		ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
		ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
		ss << "R = " << R << " ";
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ss.str().c_str() << std::endl;

		resCode = R;

		unsigned short tempU1 = U1;
		tempU1 = tempU1 >> 8;

		std::stringstream ss1;
		ss1 << "TEMP_U1 :" << tempU1;
		std::cout << "DBG: " << __FUNCTION__ << "++++++++++++++++++++++@+++++++++++++++++++++" << __LINE__ << ss1.str().c_str() << std::endl;
		// upgrade will continue if job is not performed or timeout in version-1
		if( tempU1 == 0x0001 )
		{
			std::cout <<"DBG: upgrade request in version-1 ...\t"<<__LINE__<<std::endl;
			resultJtpRes = true;
		}

		if ((res == false) || (R != 0))
		{// These case are for Job Failed and Job Timeout. Result code of Job Failed covers also Job Timeout.
			J1.jfault(RU1, RU2, R);
			//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "(jfault) The job did not complete" << std::endl;
			std::stringstream ss;
			ss << "(jfault) The job did not complete" << " ";
			ss << "jresultind, was expected" << " ";
			ss << "R = " << R;
			ss << " RU1 = " << RU1;
			ss << " RU2 = " << RU2;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << ss.str() << std::endl;

			{
				char tmpStr[512] = {0};
				std::string temp = ss.str();
				const char *tmp = temp.c_str();
				snprintf(tmpStr, sizeof(tmpStr) - 1, tmp);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				tmp = NULL;
			}

			if (RU1 ==  FIXS_CCH_JTPHandler::IMB_UPG_START_REQ)
			{
				std::cout <<"DBG: Job is not performed or timeout case for start request...\t"<<__LINE__<<std::endl;
				resultJtpRes = true;
			}
			else
			{
				std::cout <<"DBG: Job is not performed or timeout case... \t"<<__LINE__<<std::endl;
				resultJtpRes = false;
			}

			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jfault =  " << ss.str().c_str() << std::endl;

		}
		else if( RU1 ==  FIXS_CCH_JTPHandler::IMB_UPG_START_REQ )
		{
			std::cout <<"DBG: Job is performed... checking RU2 value... \t"<<__LINE__<<std::endl;
			// checking RU2 LSB ......
			{
				unsigned short res = RU2;
				res = res << 8;
				if (res != 0x00 )
				{
					resultJtpRes = false;
					resCode = FIXS_CCH_JTPClient::JOB_START_REQUEST_REJECTED;
					std::cout << "DBG : job rejected ... resCode : " << resCode << "\t" << __LINE__ << std::endl;
				}

				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "... jresultind, The data is modified" << std::endl;
				std::stringstream ss;
				ss << " RU1 = " << RU1 << " ";
				ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
				ss << "Len = " << Len << ", RLen = " << RLen << " ";
				ss << "jresultind, The data is modified";

				{
								char tmpStr[512] = {0};
								std::string temp = ss.str();
								const char *tmp = temp.c_str();
								snprintf(tmpStr, sizeof(tmpStr) - 1, tmp);
								if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
								if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
								tmp = NULL;
								std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jfault =  " << ss.str().c_str() << std::endl;
				}
			}
		}

		else if ((RU1 ==  FIXS_CCH_JTPHandler::IMBSW_START_UPGRADE) || (RU1 == FIXS_CCH_JTPHandler::IMBSW_END_UPGRADE) ||
				(RU1 ==  FIXS_CCH_JTPHandler::IPMI_START_UPGRADE) || (RU1 == FIXS_CCH_JTPHandler::IPMI_END_UPGRADE)   ||
				(RU1 ==  FIXS_CCH_JTPHandler::POWER_START_UPGRADE) || (RU1 == FIXS_CCH_JTPHandler::POWER_END_UPGRADE) ||
				(RU1 ==  FIXS_CCH_JTPHandler::FAN_START_UPGRADE) || (RU1 == FIXS_CCH_JTPHandler::FAN_END_UPGRADE))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<<std::endl;
			if ((U2 != RU2) || (Len != RLen)|| (memcmp(Msg, RMsg, Len)) != 0)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jresultind, The data is modified" << std::endl;

				std::stringstream ss;
				ss << " RU1 = " << RU1 << " ";
				ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
				ss << "Len = " << Len << ", RLen = " << RLen << " ";
				ss << "jresultind, The data is modified";

				{
					char tmpStr[512] = {0};
					std::string temp = ss.str();
					const char *tmp = temp.c_str();
					snprintf(tmpStr, sizeof(tmpStr) - 1, tmp);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_INFO);
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
					tmp = NULL;
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jfault =  " << ss.str().c_str() << std::endl;
				}

				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jfault = " << ss.str().c_str() << std::endl;
				resultJtpRes = false;
			}
		}
		else if ((U1 != RU1_temp) ||(U2 != RU2) || (Len != RLen) // Do something with the data
				|| (memcmp(Msg, RMsg, Len) != 0))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jresultind, The data is modified" << std::endl;
			std::stringstream ss;
			ss << "U1 = " << U1 << ", RU1 = " << RU1 << " ";
			ss << "U2 = " << U2 << ", RU2 = " << RU2 << " ";
			ss << "Len = " << Len << ", RLen = " << RLen << " ";
			ss << "jresultind, The data is modified";


				{
					char tmpStr[512] = {0};
					std::string temp = ss.str();
					const char *tmp = temp.c_str();
					snprintf(tmpStr, sizeof(tmpStr) - 1, tmp);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
					if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
					tmp = NULL;
				}
				resultJtpRes = false;
		}
	}
	else if (status == WAIT_OBJECT_0 + 1) // Shutdown event
	{
		J1.jtermreq();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Shutdown signalled" << std::endl;
		std::stringstream ss;
		ss << "jresultind, Shutdown signalled";

			{
				char tmpStr[512] = {0};
				std::string temp = ss.str();
				const char *tmp = temp.c_str();
				snprintf(tmpStr, sizeof(tmpStr) - 1, tmp);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				tmp = NULL;
			}
	}
	else if (status == WAIT_TIMEOUT) // Timeout event
	{
		J1.jtermreq();
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " jresultind, The call timed out" << std::endl;
		std::stringstream ss;
		ss << "jresultind, The call timed out";

			{
				char tmpStr[512] = {0};
				std::string temp = ss.str();
				const char *tmp = temp.c_str();
				snprintf(tmpStr, sizeof(tmpStr) - 1,tmp);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
				tmp = NULL;
			}
	}
	//	else //unknown error of event
	//	{
	//		J1.jtermreq();
	//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "jresultind, unknown error signalled" << std::endl;
	//		std::stringstream ss;
	//		ss << "jresultind, unknown error signalled";
	//
	//			{
	//				char tmpStr[512] = {0};
	//				std::string temp = ss.str();
	//				const char *tmp = temp.c_str();
	//				snprintf(tmpStr, sizeof(tmpStr) - 1, tmp);
	//				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	//				if (traceObj->ACS_TRA_ON())traceObj->ACS_TRA_event(1,tmpStr);
	//				tmp = NULL;
	//			}
	//		OKResult = false;
	//	}

	return resultJtpRes;

}
