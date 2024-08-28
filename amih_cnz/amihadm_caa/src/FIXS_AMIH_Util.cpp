/*
 * ACS_AMIH_Util.cpp
 *
 *  Created on: Jan 23, 2012
 *      Author: eanform
 */


#include "FIXS_AMIH_Util.h"
//#include "ACS_TRA_Trace.h"

bool AMIH_Util::stringToUlongMagazine (std::string value, unsigned long &uvalue)
{

	// convert value
	std::istringstream istr(value);
	reverseDottedDecStr(istr); //reverses the order of the dotted decimal string to match the mag addr format on the server
	if (numOfChar(istr,'.') != 3) // If not 3 dots, faulty format
	{
		return false;
	}
	if (! dottedDecToLong(istr, uvalue, 0, 15, 0, 0, 0, 15, 0, 15))
	{
		return false;
	}
	//std::cout <<"mag: "<< value.c_str()<< "umag: "<< uvalue<< std::endl;
	return true;

}

//bool AMIH_Util::ulongToStringMagazine (unsigned long uvalue, std::string &value)
bool AMIH_Util::ulongToStringMagazine (uint32_t uvalue, std::string &value)
{

	char tempValueStr[16] = {0}; //size 16, "255.255.255.255" + null character
	uint32_t tempValue;
	tempValue =  ntohl(uvalue);  //changes the representation to little-endian
	snprintf(tempValueStr, sizeof(tempValueStr) - 1, "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >> 16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
    value = tempValueStr;

	return true;

}

bool AMIH_Util::stringToUshort (std::string value, unsigned short &uvalue)
{

	std::istringstream istr(value);
	if (! checkFigures(istr))  //checks that only figures (0-9) are used in the option arg.
	{
		return false; //Usage
	}

	if (! (istr >> uvalue))   //try to convert operand to value number
	{
		return false; //Usage
	}

	if (uvalue > 25)
	{
		return false; //Usage
	}

	return true;
}

bool AMIH_Util::ushortToString (unsigned short uvalue, std::string &value)
{

	char tmp[16] = {0};
	snprintf(tmp, sizeof(tmp) - 1, "%u", uvalue);
	value = tmp;

	return true;
}
bool AMIH_Util::longlongToString (LONGLONG uvalue, std::string &value)
{
	std::stringstream ss;
	ss << uvalue;
	ss >> value;

	return true;
}

bool AMIH_Util::doubleToString (double dvalue, std::string &value)
{

	std::ostringstream sstream;
	sstream << dvalue;
	value = sstream.str();

	return true;
}

bool AMIH_Util::longToString (long uvalue, std::string &value)
{

	char tmp[16] = {0};
	snprintf(tmp,  sizeof(tmp) - 1,"%ld", uvalue);
	value = tmp;

	return true;
}
bool AMIH_Util::stringToLong (std::string value, long &uvalue)
{
	std::istringstream istr(value);
	if (! checkFigures(istr))  //checks that only figures (0-9) are used in the option arg.
	{
		return false; //Usage
	}

	char temp[16]={0};
	int strlen = value.size();

	int i;
	for (i=0;i<=strlen;i++)
	{
		temp[i]=value[i];
	}
	uvalue = atol(temp);

	return true;
}

bool AMIH_Util::stringToInt (std::string value, int &ivalue)
{
	std::istringstream istr(value);
	if (! checkFigures(istr))  //checks that only figures (0-9) are used in the option arg.
	{
		return false; //Usage
	}
	if (! (istr >> ivalue)) //try to convert operand to value number
	{
		return false; //Usage
	}

	return true;
}

std::string AMIH_Util::intToString (int ivalue)
{
	std::stringstream ss_value("");
	ss_value << ivalue; //try to convert operand to value number
	return ss_value.str();
}

bool AMIH_Util::intToUshort (int value, unsigned short &ivalue)
{
	std::ostringstream oss;
	oss << value;

	std::string res = oss.str();

	if (stringToUshort(res, ivalue))return true;
	else return false;
}

bool AMIH_Util::gprStringToHex (std::string value, int &hexvalue)
{

	std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data String Value: "<< value.c_str() <<std::endl;
	if (value.size() == 8)
	{
		std::string code = value.substr(6,8);
		std::istringstream istr(code);

		if (istr >> std::hex >> hexvalue) //try to convert operand to value number
		{
			//std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data Hex Value: "<< std::hex<< hexvalue<<std::endl;
			return true;
		}
	}

	return false;
}

unsigned short AMIH_Util::numOfChar (std::istringstream &istr, char searchChar)
{
	unsigned short numChar = 0;

	istr.clear();        // return to the beginning
	(void)istr.seekg(0); //      of the stream
	char c;

	do {
		c = istr.peek();
		if (c == searchChar)
			numChar = numChar + 1;

		(void)istr.seekg((int)(istr.tellg()) + 1);
	} while (c != EOF);

	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    // of the stream

	return numChar;
}

void AMIH_Util::reverseDottedDecStr (std::istringstream &istr)
{

	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    //      of the stream
	std::istringstream tempIstr;
	std::string tempString = "";
	char c;

	do
	{
		c = istr.get();      //get char from istringstream

		if (c == '.' || c == EOF)
		{
			(void)tempIstr.seekg(0);

			if (c == '.')
				tempIstr.str(c + tempString + tempIstr.str());
			else
				tempIstr.str(tempString + tempIstr.str());

			tempString = "";
		}
		else
			tempString = tempString + c;
	} while (c != EOF);

	istr.str(tempIstr.str());

	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    // of the stream

}

bool AMIH_Util::dottedDecToLong (std::istringstream &istr, unsigned long &value, unsigned long  lowerLimit1, unsigned long  upperLimit1, unsigned long  lowerLimit2, unsigned long  upperLimit2, unsigned long  lowerLimit3, unsigned long  upperLimit3, unsigned long  lowerLimit4, unsigned long  upperLimit4)
{
	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    //      of the stream
	char c;
	std::string strAddr = "";
	unsigned short dotNum = 0;
	value = 0;

	do {
		c = istr.get();
		if (c == '.' || c == EOF)
		{
			dotNum++;
			std::istringstream converter(strAddr);

			if (! checkFigures(converter))   //checks that only figures (0-9) are used in the option arg.
				return false;

			strAddr = "";
			unsigned long plug;
			if (! (converter >> plug)) //check if string is possible to convert to long
			{
				value = 0;
				return false;
			}

			else
			{
				if (dotNum == 1)
				{
					if (! (((plug >= lowerLimit1) && (plug <= upperLimit1)) || (plug == 255)))
					{
						value = 0;
						return false;
					}
				}

				else if (dotNum == 2)
				{
					if (! ((plug >= lowerLimit2) && (plug <= upperLimit2)))
					{
						value = 0;
						return false;
					}
				}

				else if (dotNum == 3)
				{
					if (! (((plug >= lowerLimit3) && (plug <= upperLimit3)) || (plug == 255)))  //CR GESB
					{
						value = 0;
						return false;
					}
				}

				else if (dotNum == 4)
				{
					if (! (((plug >= lowerLimit4) && (plug <= upperLimit4)) || (plug == 255)))  //CR GESB
					{
						value = 0;
						return false;
					}
				}

				value = plug | value;

				if (dotNum != 4)     // if not last byte, shift 1 byte to the left
					value = value << 8;
			}
		}

		else
			strAddr = strAddr + c;
	} while (c != EOF);

	return true;
}

bool AMIH_Util::checkFigures (std::istringstream &istr)
{
	istr.clear();			// return to the beginning
	(void)istr.seekg(0);	// of the stream
	char c = istr.get();	// get first char in the istringstream

	while (c != EOF)
	{
		if ((c < '0') || (c > '9'))
			return false;

		c = istr.get();
	}

	istr.clear();		// return to the beginning
	(void)istr.seekg(0);	// of the stream

	return true;
}

bool AMIH_Util::stringToUpper (std::string& stringToConvert)
{

	//Changes each element of the string to upper case
	std::string convertedString = stringToConvert;

	for(unsigned int i = 0; i < stringToConvert.length(); i++)
	{
		convertedString[i] = toupper(stringToConvert[i]);
	}

	stringToConvert = convertedString;    //return the converted string

	return true;
}


bool AMIH_Util::createDir (const char* dirpath, DWORD& err)
{
	UNUSED(dirpath);
	UNUSED(err);
	return false;
	/*
	char directory[MAX_PATH] = {0};
	char createdir[MAX_PATH] = {0};
	int i = 0;
	BOOL test = TRUE;
	struct _stat buffer;
	strncpy(directory, dirpath, sizeof(directory) - 1);
	int dirStatus = _stat(directory, &buffer);

	if ((errno == ENOENT) && (dirStatus == -1))
	{
		for (i=0;i<3;i++)
		{
			createdir[i] = directory[i];
		}

		while (directory[i] != '\0')
		{
			createdir[i] = directory[i];
			createdir[i+1]='\0';
			if (directory[i] == '\\')
			{
				createdir[i+1] = '\0';
				test = CreateDirectory(createdir, 0);
				if (!test)
				{
					err = GetLastError();
					if (err != ERROR_ALREADY_EXISTS)
						return false;
				}
			}
			i++;
		}

		test = CreateDirectory(createdir, 0);  //Creating last part of the directory path
		if (!test)
		{
			err = GetLastError();
			if (err == ERROR_ALREADY_EXISTS)
				test = TRUE;
		}
	}

	if (test)
		return true;
	else
	*/
}

void AMIH_Util::getCurrentTime(std::string& dateStr, std::string& timeStr)
{
	time_t longTime;
	time(&longTime);  // Get the current time
	struct tm* today = localtime(&longTime); // Convert to local time

	// Retrieve time information
	char dateChar[256] = {0};
	strftime(dateChar, sizeof (dateChar) - 1, "%Y-%m-%d", today);

	char timeChar[256] = {0};
	strftime(timeChar, sizeof (timeChar) - 1, "%H:%M:%S ", today);

	dateStr = dateChar;
	timeStr = timeChar;
}

std::string AMIH_Util::getCurrentTimeInSeconds()
{
	long timeinseconds;
	int seconds;
	int minutes;
	int hours;
	std::string secInStr;

	time_t longTime;
	time(&longTime);  // Get the current time
	struct tm* today = localtime(&longTime); // Convert to local time

	seconds = today->tm_sec;
	minutes = today->tm_min;
	hours = today->tm_hour;

	timeinseconds = (hours*3600)+(minutes*60)+seconds;

	AMIH_Util::longToString(timeinseconds,secInStr);

	return secInStr;
}

bool AMIH_Util::CopyFileToDataDisk(std::string & pathFile, std::string & destFile)
{
	bool fileAlreadyPresentInK = false;
	struct stat st;
	off_t size;
	//check if exist
	if(stat(destFile.c_str(),&st) != 0){
		fileAlreadyPresentInK = false;
		size = st.st_size;
	}
	else fileAlreadyPresentInK = true;


	if (fileAlreadyPresentInK == false)
	{
		std::cout << "DBG: CopyFile("<<pathFile.c_str()<<","<<destFile.c_str()<<")"<<std::endl;


		if (AMIH_Util::CopyFile(pathFile.c_str(),destFile.c_str(),false,size) == false )
		{
			return false;
		}

		return true;
	}
	else
	{
		return true;
	}
}

std::string AMIH_Util::replaceSubstr( const std::string& content, const std::string& instance, const std::string& replacement )
{
	std::ostringstream new_content;

    if ( instance.empty() || content.empty() || content.find(instance) == std::string::npos || instance == replacement )
	{
        return content;
    }

    if (instance.size() == 1)
	{
        for (std::string::const_iterator i = content.begin(); i != content.end(); ++i)
		{
            if ( *i == instance[0] )
			{
                new_content << replacement;
            }
			else
			{
                new_content << *i;
            }
        }
    }
	else
	{
        std::string remaining( content );
        const size_t instance_size( instance.size() );
        for ( size_t start_pos_of_instance; ( start_pos_of_instance = remaining.find(instance) ) != std::string::npos; )
		{
            new_content.write( &remaining[0], start_pos_of_instance); // was using remaining.substr()
            new_content << replacement;
            //remaining.erase(0, start_pos_of_instance + instance_size);
            std::ostringstream temp;
            temp.write(&remaining[ start_pos_of_instance + instance_size], remaining.size() - (start_pos_of_instance + instance_size) );
            remaining = temp.str();
            temp.clear();
        }
        new_content << remaining;
    }

    return new_content.str();
}

std::string AMIH_Util::getEnvironmentType(AMIH_Util::EnvironmentType env)
{
	std::string envStr;
	switch(env)
	{
		case AMIH_Util::SINGLECP_NOTCBA:
			envStr = "SINGLECP_NOTCBA";
			break;
		case AMIH_Util::SINGLECP_CBA:
			envStr = "SINGLECP_CBA";
			break;
		case AMIH_Util::MULTIPLECP_NOTCBA:
			envStr = "MULTIPLECP_NOTCBA";
			break;
		case AMIH_Util::MULTIPLECP_CBA:
			envStr = "MULTIPLECP_CBA";
			break;
		default:
			envStr = "UNKNOWN";
	}
	return envStr;
}

AMIH_Util::EnvironmentType AMIH_Util::isRightIptEnvironment(void)
{
	bool isCBA = false;
	bool multipleCPSystem = false;

	UNUSED(isCBA);
	UNUSED(multipleCPSystem);

	return UNKNOWN;
	/*
	ACS_CS_API_NS::CS_API_Result result = ACS_CS_API_NS::Result_Failure;
	ACS_CS_API_CommonBasedArchitecture::ArchitectureValue nodeArchitecture;
	if (ACS_CS_API_NetworkElement::getNodeArchitecture(nodeArchitecture) != ACS_CS_API_NS::Result_Success)
	{
		if (ACS_TRA_ON(traceUtil))
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,
				"[%s@%d] No reply from Configuration Service.GetLastError(): %u",
				__FUNCTION__, __LINE__,GetLastError());
			ACS_TRA_event(&traceUtil, tmpStr);
		}
		//std::cout << "No reply from Configuration Service" << std::endl;
		return UNKNOWN;
	}
	if (nodeArchitecture == ACS_CS_API_CommonBasedArchitecture::ArchitectureValue::SCX_CBA)
	{
		isCBA = true;
	}
	result = ACS_CS_API_NetworkElement::isMultipleCPSystem(multipleCPSystem);
	if (result == ACS_CS_API_NS::Result_Success)
	{
		if (multipleCPSystem && isCBA)
		{
			if (ACS_TRA_ON(traceUtil))
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,
					"[%s@%d] Multiple-CP,CBA Environment ",
					__FUNCTION__, __LINE__);
				ACS_TRA_event(&traceUtil, tmpStr);
			}
			return MULTIPLECP_CBA;
		}
		else if (multipleCPSystem && (!isCBA))
		{
			if (ACS_TRA_ON(traceUtil))
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,
					"[%s@%d] Multiple-CP,Not CBA Environment ",
					__FUNCTION__, __LINE__);
				ACS_TRA_event(&traceUtil, tmpStr);
			}
			return MULTIPLECP_NOTCBA;
		}
		else if((!multipleCPSystem) && isCBA)
		{
			if (ACS_TRA_ON(traceUtil))
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,
					"[%s@%d] Single-CP,CBA Environment ",
					__FUNCTION__, __LINE__);
				ACS_TRA_event(&traceUtil, tmpStr);
			}
			return SINGLECP_CBA;
		}
		else
		{
			if (ACS_TRA_ON(traceUtil))
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,
					"[%s@%d] Single-CP,Not CBA not supported by IPTB",
					__FUNCTION__, __LINE__);
				ACS_TRA_event(&traceUtil, tmpStr);
			}
			return SINGLECP_NOTCBA; // single-CP, NO CBA not supported by IPTB
		}
	}
	else
	{
		if (ACS_TRA_ON(traceUtil))
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,
				"[%s@%d] Failed to get multiple CP system.GetLastError(): %u",
				__FUNCTION__, __LINE__,GetLastError());
			ACS_TRA_event(&traceUtil, tmpStr);
		}
		//std::cout << "Failed to get multiple CP system" << std::endl;
		return UNKNOWN;
	}
	*/
}

//-----------------------
//	Create symbolic link
// An alternative way - system ("junction -s <link folder> <dir>");
//-----------------------
int AMIH_Util::createSymbolicLink(const std::string& link, const std::string& dir)
{
	UNUSED(link);
	UNUSED(dir);
	return 0;
	/*
	std::string tdir;
	if (dir[0] == '/' && dir[1] == '?')
	{
		tdir = dir;
	}
	else
	{
		tdir = "/??/";
		TCHAR fullDir[MAX_PATH];
		LPTSTR filePart;
		DWORD len = GetFullPathName(dir.data(), MAX_PATH, fullDir, &filePart);
		if (len == 0)
		{
			std::cout << "DBG: " << "Failed to get full path name for file '" << dir << "'.";
			return 1;
		}

		DWORD ret = GetFileAttributes(fullDir);
		if (ret == INVALID_FILE_ATTRIBUTES)
		{
			std::cout << "DBG: " << "Failed to get file attributes for file '" << fullDir << "'.";
			return 1;
		}
		if (!(ret & FILE_ATTRIBUTE_DIRECTORY))
		{
			std::cout << "DBG: " << "Target '" << dir << "' is not a directory.";
			return 1;
		}
		tdir += fullDir;
	}

	BOOL ok;
	ok = CreateDirectory(link.data(), NULL);
	if ((ok == FALSE) && (GetLastError() != ERROR_ALREADY_EXISTS))
	{
		std::cout << "DBG: " << "Failed to create symbolic link '" << link << "'. GetLastError = " << GetLastError() << std::endl;
		return 1;
	}

	HANDLE hLink = CreateFile(
						link.data(),
						GENERIC_READ | GENERIC_WRITE,
						0,
						0,
						OPEN_EXISTING,
						FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
						0
						);

	if (hLink == INVALID_HANDLE_VALUE)
	{
		std::cout << "DBG: " << "Failed to create symbolic link '" << link << "'. GetLastError = " << GetLastError() << std::endl;
		return 1;
	}

	char szBuff[MAXIMUM_REPARSE_DATA_BUFFER_SIZE] = { 0 };
	TMN_REPARSE_DATA_BUFFER& rdb = *(TMN_REPARSE_DATA_BUFFER*)szBuff;

	// If the destination path contains network name it must be removed
	// from the path name
	if (tdir.substr(4, 2) == "\\\\")
	{
		size_t pos = tdir.find_first_of('\\', 6);
		tdir[pos + 2] = ':';
		tdir = tdir.erase(3, pos - 3);
	}

	const std::wstring& wDestMountPoint = AMIH_Util::mbsToWcs(tdir);
	const WORD nDestMountPointBytes = (WORD)wDestMountPoint.size() * 2;

	rdb.ReparseTag           = IO_REPARSE_TAG_MOUNT_POINT;
	rdb.ReparseDataLength    = nDestMountPointBytes + 12;
	rdb.Reserved             = 0;
	rdb.SubstituteNameOffset = 0;
	rdb.SubstituteNameLength = nDestMountPointBytes;
	rdb.PrintNameOffset      = nDestMountPointBytes + 2;
	rdb.PrintNameLength      = 0;
	(void)wcscpy(rdb.PathBuffer, wDestMountPoint.data());
	DWORD size = rdb.ReparseDataLength + TMN_REPARSE_DATA_BUFFER_HEADER_SIZE;
	DWORD bytes;

	ok = DeviceIoControl(
					hLink,
					FSCTL_SET_REPARSE_POINT,
					(LPVOID)&rdb,
					size,
					NULL,
					0,
					&bytes,
					0
					);
	if (ok == FALSE)
	{
		(VOID)CloseHandle(hLink);

		std::cout << "DBG: " << "Failed to set reparse point.";
		return 1;
	}

	(VOID)CloseHandle(hLink);
*/
}

std::string AMIH_Util::ulongToStringIP (unsigned long ipaddress)
{
	char tempValueStr[16] = {0};
	uint32_t tempValue = ipaddress;
	snprintf(tempValueStr, sizeof(tempValueStr), "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >> 16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
    std::string IP = tempValueStr;
	return IP;
}


bool AMIH_Util::isFlashValue (std::string value)
{
	if (value.size() < 5) return false;
	char code = value[5];

	if (code == '0')
		return true;

	return false;
}

bool AMIH_Util::isNetworkValue (std::string value)
{
	if (value.size() < 5) return false;
	char code = value[5];

	if (code == '1')
		return true;

	return false;
}

std::string AMIH_Util::GetAPHostName()
{
    char name[255] = {0};

	{
		if( gethostname( name, sizeof(name)) == 0)
		{
			//OK
		/*	if (ACS_TRA_ON(traceUtil))
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] read hostname %s", __FUNCTION__, __LINE__, name);
				ACS_TRA_event(&traceUtil, tmpStr);
			}
		*/
		}
		else
		{
		/*	if (ACS_TRA_ON(traceUtil))
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Failed to retrieve the hostname", __FUNCTION__, __LINE__);
				ACS_TRA_event(&traceUtil, tmpStr);
			}
		*/
		}
	}
    return name;
}

bool AMIH_Util::stringTolonglong (std::string value, LONGLONG &ivalue)
{
	std::istringstream istr(value);
	if (! checkFigures(istr))  //checks that only figures (0-9) are used in the option arg.
	{
		return false; //Usage
	}

	if (! (istr >> ivalue))   //try to convert operand to value long long
	{
		return false; //Usage
	}
	return true;
}

LONGLONG AMIH_Util::getEpochTimeInSeconds()
{
	LONGLONG timeinseconds;
	std::string secInStr;

	time_t sec;
	sec = time(NULL);  // Get the current time in epch sec

	timeinseconds = static_cast<LONGLONG> (sec);

	return timeinseconds;
}

// calculate number of digit present till decimal point
int AMIH_Util::calculateDigit(double x)
{
	int count =0;
	while(x > 1)
	{
		x = x/10;
		count++;
	}
	return count;
}

bool AMIH_Util::gprScxStringToHex (std::string value, int &hexvalue)
{

	std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data String Value: "<< value.c_str() <<std::endl;
	if (value.size() == 23)
	{
		std::string code = value.substr(21,22);
		std::istringstream istr(code);

		if (istr >> std::hex >> hexvalue) //try to convert operand to value number
		{
			//std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data Hex Value: "<< std::hex<< hexvalue<<std::endl;
			return true;
		}
	}

	return false;
}


bool AMIH_Util::CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists, long int size)
{
   if ( bFailedIfExists && AMIH_Util::PathFileExists(szDestFile) )
   {
      return false;
   }

   FILE* pSourceFile;
   FILE* pDestFile;
   unsigned char copy_buffer[size];
   int byte_count_read = 0;
   int byte_count_written = 0;

   pSourceFile = ACE_OS::fopen(szSourceFile, "rb");
   if ( !pSourceFile )
   {
      return false;
   }

   pDestFile = ACE_OS::fopen(szDestFile, "wb");
   if ( !pDestFile )
   {
	   ACE_OS::fclose(pSourceFile);
      return false;
   }

   for(;;)
   {
      byte_count_read = ACE_OS::fread(copy_buffer, 1, size, pSourceFile);
      if ( byte_count_read > 0 )
      {
         byte_count_written = ACE_OS::fwrite(copy_buffer, 1, byte_count_read, pDestFile);

         if ( byte_count_written != byte_count_read )
         {
        	 ACE_OS::fclose(pSourceFile);
        	 ACE_OS::fclose(pDestFile);
            return false;
         }
      }

      if ( ::feof(pSourceFile) )
         break;
   }

   ACE_OS::fclose(pSourceFile);
   ACE_OS::fclose(pDestFile);

   return true;
}




bool AMIH_Util::CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists)
{
   int size = 0;
   struct stat st;

   int ris=stat(szSourceFile,&st);

   if (ris<0)
   {
	   printf("file not found!");
	   return false;
   }

   size=st.st_size;

   if ( bFailedIfExists && PathFileExists(szDestFile) )
   {
      return false;
   }

   FILE* pSourceFile;
   FILE* pDestFile;

   unsigned char *copy_buffer = NULL;

   copy_buffer=(unsigned char*)malloc(sizeof(unsigned char)*(size+1));

   memset(copy_buffer,0,size+1);

   int byte_count_read = 0;
   int byte_count_written = 0;

   fflush(stdout);

   pSourceFile = fopen(szSourceFile, "rb");

   if ( !pSourceFile )
   {
	   free(copy_buffer);
	   copy_buffer=NULL;
      return false;
   }

   pDestFile = fopen(szDestFile, "wb");

   if ( !pDestFile )
   {
	   fclose(pSourceFile);
	   free(copy_buffer);
	   copy_buffer=NULL;
      return false;
   }

   for(;;)
   {
      byte_count_read = fread(copy_buffer, 1, size, pSourceFile);

      if ( byte_count_read > 0 )
      {

         byte_count_written = fwrite(copy_buffer, 1, byte_count_read, pDestFile);

         if ( byte_count_written != byte_count_read )
         {

        	 fclose(pSourceFile);
        	 pSourceFile=NULL;
        	 fclose(pDestFile);
        	 pDestFile=NULL;
        	 free(copy_buffer);
        	 copy_buffer=NULL;
            return false;
         }
      }

      if (::feof(pSourceFile) )
      {
         break;
      }
   }

   if(copy_buffer!=NULL)
   {
	   free(copy_buffer);
	   copy_buffer=NULL;
   }

   fflush(stdout);

   if(pSourceFile!=NULL)
   {
	   fclose(pSourceFile);
	   pSourceFile=NULL;
   }

   fflush(stdout);

   if(pDestFile!=NULL)
   {
	   fclose(pDestFile);
	   pDestFile=NULL;
   }

   fflush(stdout);

   return true;
}


bool AMIH_Util::PathFileExists(const char* file)
{
	struct stat st;
		//check if exist
		if(stat(file,&st) != 0)	return false;
		else return true;

}


int AMIH_Util::findFile(char* path, char* type, std::vector<std::string> &result)
{
	fflush(stdout);
	//static int 	recursive_call = 0;
	DIR		*dp;
	struct dirent	*dirp;
	struct stat 	statbuf;
	int 		ret = 0;

	if((dp = opendir(path)) == NULL){

		printf("\nCan't read directory %s\n", path);

		return ERROR;

	}
	//opendir OK

	char* new_path=NULL;
	bool exit=false;
	int resu=0;

	while(((dirp = readdir(dp)) != NULL) &&  !exit)
	{
		if((strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
		|| (strlen(type)>strlen(path)))//CRASH!!!!!!!!!!!!!
		{
			continue;	//ignore dot and dot-dot
		}
		new_path = (char*) calloc((strlen(path) + strlen(dirp->d_name) + 2), sizeof(char));

		strcat(new_path, path);
		strcat(new_path, "/");
		strcat(new_path, dirp->d_name);


		if(lstat(new_path, &statbuf) < 0)
		{
			exit=true;
			free(new_path);
			new_path=NULL;
			printf("\nlstat error on path %s", new_path);
			resu= ERROR;
			continue;
		}


		if(S_ISLNK(statbuf.st_mode))
		{
			printf("\nlink path %s found! skip it\n ", new_path);
			free(new_path);
			new_path=NULL;
			continue;

		}

		if(S_ISREG(statbuf.st_mode))
		{
			std::string s_new_path (new_path);

			std::string s_type (type);

			if(s_new_path.substr(s_new_path.length()-s_type.length(), s_type.length()) == s_type)
			{
				result.push_back(std::string(new_path));
			}

		}
		else
		{
			if(S_ISDIR(statbuf.st_mode)){

				if((ret = findFile(new_path, type, result)) < 0)
				{
					//free(new_path);
					resu=ret;
					exit=true;
				}

			}
		}

		free(new_path);
		new_path=NULL;


	}// end-while

	//close directory

	if(closedir(dp) < 0){

		printf("\nclosedir error on path %s\n", path);

	}

	free(dirp);


	return resu;

}




int AMIH_Util::removeFolder(const char* _fullpath){

	DIR		*dp;
	struct dirent	*dirp;
	struct stat 	statbuf;
	char		*new_fullpath;
	int 		ret;

	if (stat(_fullpath,&statbuf) != 0) return 0;

	if((dp = opendir(_fullpath)) == NULL)
	{
		return OPENDIR_ERROR;
	}

	//opendir OK

	while((dirp = readdir(dp)) != NULL){

		if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue;	//ignore dot and dot-dot

		new_fullpath = (char*) calloc((strlen(_fullpath) + strlen(dirp->d_name) + 2), sizeof(char));

		strcat(new_fullpath, _fullpath);
		strcat(new_fullpath, "/");
		strcat(new_fullpath, dirp->d_name);

		if(stat(new_fullpath, &statbuf) != 0)
		{
			free(new_fullpath);
			new_fullpath = NULL;
			continue;
		}

		if(S_ISDIR(statbuf.st_mode)){

		//the current file is a directory
			//it needs remove the entire content
			if((ret = removeFolder(new_fullpath)) < 0)
			{
				free(new_fullpath);
				//close directory
				if(closedir(dp) < 0){
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "closedir "<< std::endl;
					return CLOSEDIRR_ERROR;
				}
				return ret;
			}
		}

		//remove file
		else
			if(remove(new_fullpath) < 0){
				free(new_fullpath);
				closedir(dp);
				return REMOVE_ERROR;
			}else std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Remove file "<< std::endl;

		free(new_fullpath);

	}// end-while

	//close directory
	if(closedir(dp) < 0){
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "closedir "<< std::endl;
		return CLOSEDIRR_ERROR;
	}

	//remove directory
	if(remove(_fullpath) < 0){
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "remove "<< std::endl;
		return REMOVE_ERROR;
	}

	return 0;

}// end-removeFolder

bool AMIH_Util::CheckExtention(const char* file, const char* type_file)
{
	bool result;
	result = false;
	char* fileCheck = strndup(file+strlen(file)-strlen(type_file), strlen(type_file));
	if (strcmp(fileCheck, type_file) == 0)
	{
		result = true;
	}
	free(fileCheck);
	return result;
}


void AMIH_Util::stringIpToUlong(const char *ip, unsigned long &uvalue)
{
	unsigned long temp = inet_addr(ip);
	uvalue = htonl(temp);
}


void AMIH_Util::findAndReplace( string &source, const string find, string replace )
{
    size_t j;
    for ( ; (j = source.find( find )) != string::npos ; )
    {
        source.replace( j, find.length(), replace );
    }
}


void AMIH_Util::trim(string& str)
{
	string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
}

void AMIH_Util::findAndRemove( string &source, const string find)
{
	size_t j;
	for ( ; (j = source.find( find )) != string::npos ; )
	{
		source.erase(j,1);
	}
}


