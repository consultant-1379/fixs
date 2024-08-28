
#include "xcountls_Util.h"
#include "acs_apgcc_omhandler.h"

bool XCOUNTLS_Util::stringToUlong (std::string value, unsigned long &uvalue)
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

bool XCOUNTLS_Util::ulongToStringMagazine (unsigned long uvalue, std::string &value)
{

	char tempValueStr[16] = {0}; //size 16, "255.255.255.255" + null character
	uint32_t tempValue;
	tempValue =  ntohl(uvalue);  //changes the representation to little-endian
	snprintf(tempValueStr, sizeof(tempValueStr) - 1, "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >> 16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
    value = tempValueStr;

	return true;

}

bool XCOUNTLS_Util::stringToUshort (std::string value, unsigned short &uvalue)
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

	if (uvalue > 28 || uvalue == 27)
	{
		return false; //Usage
	}

	return true;
}

bool XCOUNTLS_Util::ushortToString (unsigned short uvalue, std::string &value)
{

	char tmp[16] = {0};
	snprintf(tmp, sizeof(tmp) - 1, "%u", uvalue);
	value = tmp;

	return true;
}
bool XCOUNTLS_Util::ulonglongToString (ULONGLONG uvalue, std::string &value)
{
	std::stringstream ss;
	ss << uvalue;
	ss >> value;

	return true;
}

bool XCOUNTLS_Util::doubleToStringRate(double dvalue, std::string &value)
{

	std::ostringstream sstream;
	sstream.precision(15);
	sstream.setf (ios::fixed,ios::floatfield );
	sstream.setf ( ios::showbase );
	sstream << dvalue;
	//snprintf(buffer, 17, "%s", sstream.str());
	value = sstream.str();
	//value=std::string(buffer);
	value.resize(17);
	return true;
}

bool XCOUNTLS_Util::doubleToString (double dvalue, std::string &value)
{

	std::ostringstream sstream;
	sstream << dvalue;
	value = sstream.str();

	return true;
}

bool XCOUNTLS_Util::longToString (long uvalue, std::string &value)
{

	char tmp[16] = {0};
	snprintf(tmp,  sizeof(tmp) - 1,"%ld", uvalue);
	value = tmp;

	return true;
}
bool XCOUNTLS_Util::stringToLong (std::string value, long &uvalue)
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

bool XCOUNTLS_Util::stringToInt (std::string value, int &ivalue)
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

std::string XCOUNTLS_Util::intToString (int ivalue)
{
	std::stringstream ss_value("");
	ss_value << ivalue; //try to convert operand to value number
	return ss_value.str();
}

bool XCOUNTLS_Util::intToUshort (int value, unsigned short &ivalue)
{
	std::ostringstream oss;
	oss << value;

	std::string res = oss.str();

	if (stringToUshort(res, ivalue))return true;
	else return false;
}

bool XCOUNTLS_Util::gprStringToHex (std::string value, int &hexvalue)
{

	std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data String Value: "<< value.c_str() <<std::endl;
	if (value.size() == 8)
	{
		std::string code = value.substr(6,8);
		std::istringstream istr(code);

		if (istr >> std::hex >> hexvalue) //try to convert operand to value number
		{
//			std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data Hex Value: "<< std::hex<< hexvalue<<std::endl;
			return true;
		}
	}

	return false;
}

unsigned short XCOUNTLS_Util::numOfChar (std::istringstream &istr, char searchChar)
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

void XCOUNTLS_Util::reverseDottedDecStr (std::istringstream &istr)
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

bool XCOUNTLS_Util::dottedDecToLong (std::istringstream &istr, unsigned long &value, unsigned long  lowerLimit1, unsigned long  upperLimit1, unsigned long  lowerLimit2, unsigned long  upperLimit2, unsigned long  lowerLimit3, unsigned long  upperLimit3, unsigned long  lowerLimit4, unsigned long  upperLimit4)
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

bool XCOUNTLS_Util::checkFigures (std::istringstream &istr)
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


bool XCOUNTLS_Util::stringToUpper (std::string& stringToConvert)
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

void XCOUNTLS_Util::getCurrentTime(std::string& dateStr, std::string& timeStr)
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

std::string XCOUNTLS_Util::getCurrentTimeInSeconds()
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

	XCOUNTLS_Util::longToString(timeinseconds,secInStr);

	return secInStr;
}

bool XCOUNTLS_Util::CopyFileToDataDisk(std::string & pathFile, std::string & destFile)
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


		if (XCOUNTLS_Util::CopyFile(pathFile.c_str(),destFile.c_str(),false,size) == false )
		{

			//error
	/*		if (ACS_TRA_ON(traceUtil))
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,
					"[%s@%d] [swInstall] Copy failed. Source file: %s. Destination file: %s. GetLastError(): %u",
					__FUNCTION__, __LINE__, pathFile.c_str(), destFile.c_str(), GetLastError());
				//debugging, std::cout << "DBG: " << tmpStr << std::endl;
				// if (ACS_TRA_ON(traceUtil))
			//ACS_TRA_event(&traceUtil, tmpStr);
			}
	*/
			return false;
		}

	/*	if (ACS_TRA_ON(traceUtil))
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1,
				"[%s@%d] [swInstall] Copy from:{%s} to:{%s}",
				__FUNCTION__, __LINE__, pathFile.c_str(), destFile.c_str());
			//debugging, std::cout << "DBG: " << tmpStr << std::endl;
			// if (ACS_TRA_ON(traceUtil))
			  //ACS_TRA_event(&traceUtil, tmpStr);
		}
	*/

		return true;
	}
	else
	{
		return true;
	}
}

//--------------------------------------------------------------
//	Convert multibyte character string to wide character string
//--------------------------------------------------------------
std::wstring XCOUNTLS_Util::mbsToWcs(const std::string& str)
{
	std::wstring tstr(str.size(), L' ');
	std::wstring::iterator titer = tstr.begin();
	for (std::string::const_iterator iter = str.begin(); iter != str.end(); iter++)
	{
		*titer = std::cout.widen(*iter);
		titer++;
	}
	return tstr;
}


std::string XCOUNTLS_Util::ulongToStringIP (unsigned long ipaddress)
{
	char tempValueStr[16] = {0};
	uint32_t tempValue = ipaddress;
	snprintf(tempValueStr, sizeof(tempValueStr), "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >> 16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
    std::string IP = tempValueStr;
	return IP;
}

bool XCOUNTLS_Util::isFlashValue (std::string value)
{
	if (value.size() < 5) return false;
	char code = value[5];

	if (code == '0')
		return true;

	return false;
}

bool XCOUNTLS_Util::isNetworkValue (std::string value)
{
	if (value.size() < 5) return false;
	char code = value[5];

	if (code == '1')
		return true;

	return false;
}

std::string XCOUNTLS_Util::GetAPHostName()
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

bool XCOUNTLS_Util::stringToulonglong (std::string value, ULONGLONG &ivalue)
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

XCOUNTLS_Util::ULONGLONG XCOUNTLS_Util::getEpochTimeInSeconds()
{
	ULONGLONG timeinseconds;
	std::string secInStr;

	time_t sec;
	sec = time(NULL);  // Get the current time in epch sec

	timeinseconds = static_cast<ULONGLONG> (sec);

	return timeinseconds;
}

// calculate number of digit present till decimal point
int XCOUNTLS_Util::calculateDigit(double x)
{
	int count =0;
	while(x > 1)
	{
		x = x/10;
		count++;
	}
	return count;
}

bool XCOUNTLS_Util::CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists, long int size)
{
   if ( bFailedIfExists && XCOUNTLS_Util::PathFileExists(szDestFile) )
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




bool XCOUNTLS_Util::CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists)
{
   int size = 0;
   struct stat st;

   int ris=stat(szSourceFile,&st);

   if (ris<0)
   {
	   //***************************************
	   //printf("file not found!");
	   //***************************************
	   return false;
   }

   size=st.st_size;

   if ( bFailedIfExists && PathFileExists(szDestFile) )
   {
      return false;
   }

   FILE* pSourceFile;
   FILE* pDestFile;

   unsigned char *copy_buffer=NULL;

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


bool XCOUNTLS_Util::PathFileExists(const char* file)
{
	struct stat st;
		//check if exist
		if(stat(file,&st) != 0)	return false;
		else return true;

}


int XCOUNTLS_Util::findFile(char* path, char* type, std::vector<std::string> &result)
{
	fflush(stdout);
	//static int 	recursive_call = 0;
	DIR		*dp;
	struct dirent	*dirp;
	struct stat 	statbuf;
	int 		ret = 0;

	if((dp = opendir(path)) == NULL){
		//****************DEBUG***********************
		//printf("\nCan't read directory %s\n", path);
		//***************************************
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
			//***************************************
			//printf("\nlstat error on path %s", new_path);
			//***************************************
			resu= ERROR;
			continue;
		}


		if(S_ISLNK(statbuf.st_mode))
		{
			//***************************************
			//printf("\nlink path %s found! skip it\n ", new_path);
			//***************************************
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


	if(new_path!=NULL)
	{
		free(new_path);
		new_path=NULL;
	}

	//close directory

	if(closedir(dp) < 0){
		//***************************************
		//printf("\nclosedir error on path %s\n", path);
		//***************************************

	}

	free(dirp);


	return resu;

}




int XCOUNTLS_Util::removeFolder(const char* _fullpath){

	DIR		*dp = NULL;
	struct dirent	*dirp = NULL;
	struct stat 	statbuf;
	char		*new_fullpath = NULL;
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
					//*****************DEBUG**********************
					//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "closedir "<< std::endl;
					//***************************************
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
			}else
				//***************DEBUG************************
				//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Remove file "<< std::endl;
				//***************************************

		free(new_fullpath);

	}// end-while

	//close directory
	if(closedir(dp) < 0){
		//*************DEBUG**************************
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "closedir "<< std::endl;
		//***************************************
		return CLOSEDIRR_ERROR;
	}

	//remove directory
	if(remove(_fullpath) < 0){
		//***************DEBUG************************
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "remove "<< std::endl;
		//***************************************
		return REMOVE_ERROR;
	}

	return 0;

}// end-removeFolder

bool XCOUNTLS_Util::CheckExtention(const char* file, const char* type_file)
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


void XCOUNTLS_Util::stringIpToUlong(const char *ip, unsigned long &uvalue)
{
	unsigned long temp = inet_addr(ip);
	uvalue = htonl(temp);
}


void XCOUNTLS_Util::findAndReplace( std::string &source, const std::string find, std::string replace )
{
    size_t j;
    for ( ; (j = source.find( find )) != std::string::npos ; )
    {
        source.replace( j, find.length(), replace );
    }
}


void XCOUNTLS_Util::trim(std::string& str)
{
	std::string::size_type pos = str.find_last_not_of(' ');
  if(pos != std::string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != std::string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
}

void XCOUNTLS_Util::findAndRemove( std::string &source, const std::string find)
{
	size_t j;
	for ( ; (j = source.find( find )) != std::string::npos ; )
	{
		source.erase(j,1);
	}
}



int XCOUNTLS_Util::filecopy(const std::string &source, const std::string &dest) {

	FILE *fp1=NULL, *fp2=NULL;
	int c ;
	fp1 = fopen( source.c_str(),  "r" );	/* open for reading */
	fp2 = fopen( dest.c_str(), "w" ) ; 		/* open for writing */
	if ( fp1 == NULL )      				/* check does file exist etc */
	{
		//*********DEBUG*************************
		//std::cout << "File source not found " << std::endl;
		//***************************************
		if ( fp2 != NULL ) {
			fclose (fp2);
		}

		return -1;
	}
	else if ( fp2 == NULL )
	{
		//*********DEBUG*************************
		//std::cout << "File destination error ! " << std::endl;
		//***************************************
		if ( fp1 != NULL ) {
			fclose (fp1);
		}

		return -1;
	}
	else   /* both files O.K. */
	{
		c = getc(fp1) ;
		while ( c != EOF)
		{
			putc( c,  fp2);    /* copy to prog.old */
			c =  getc( fp1 ) ;
		}
		fclose ( fp1 );   /* Now close files */
		fclose ( fp2 );
		//*********DEBUG*************************
		//std::cout << "File copied ! " << std::endl;
		//***************************************
	}
	return 0;
}


std::string XCOUNTLS_Util::getPcSectionName(unsigned long umagazine, unsigned short uslot)
{
	std::string section;
	std::string magazine;
	std::string slot;

	//NOTE !!!!!
	//with boost is not possibile use "." in the shelf number
	//error: ptree is too deep

	char magazine_addr_str [16] = {0};
	uint32_to_ip_format(magazine_addr_str, umagazine, '_');

	magazine = std::string(magazine_addr_str);

	//if (XCOUNTLS_Util::ulongToStringMagazine (umagazine, magazine))
	{
		if (ushortToString (uslot, slot))
		{
			section.append("BS:");
			section.append(magazine);
			section.append(":");
			section.append(slot);
		}
	}
	return section;
}


int XCOUNTLS_Util::uint32_to_ip_format (char (& value_ip) [16], uint32_t value, char separator) {

	uint8_t ip_fields [4];

	XCOUNTLS_Util::uint32_to_ip_fields(ip_fields, value);

	::snprintf(value_ip, (sizeof(value_ip)/sizeof(*(value_ip))), "%u%c%u%c%u%c%u",
			ip_fields[0], separator, ip_fields[1], separator, ip_fields[2], separator, ip_fields[3]);

	return 0;
}

int XCOUNTLS_Util::uint32_to_ip_fields (uint8_t (& ip_fields) [4], uint32_t value) {
	uint8_t * p = reinterpret_cast<uint8_t *>(&value);

	for (size_t i = 0; i < (sizeof(ip_fields)/sizeof(*(ip_fields))); ip_fields[i++] = *p++) ;

	return 0;
}

bool XCOUNTLS_Util::ulongToString (unsigned long uvalue, std::string &value)
{
	char tmp[16] = {0};
	snprintf(tmp,  sizeof(tmp) - 1,"%lu", uvalue);
	value = tmp;
	return true;
}

bool XCOUNTLS_Util::getImmAttributeInt(const std::string &strDn, const std::string &strAttr, int *iVal)
{
	ACS_CC_ReturnType nResult;

	ACS_CC_ImmParameter Param;

	OmHandler omHandler;

	char *name_attrPath;

	nResult = ACS_CC_FAILURE;
	name_attrPath = NULL;

	nResult = omHandler.Init();

	if (nResult != ACS_CC_SUCCESS)
	{
		//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
	}

	if(nResult == ACS_CC_SUCCESS)
	{
		name_attrPath = const_cast<char*>(strAttr.c_str());

		Param.attrName = name_attrPath;

		nResult = omHandler.getAttribute(strDn.c_str(), &Param );

		if ( nResult != ACS_CC_SUCCESS ){
			//std::cout << "ERROR: Param FAILURE!!!\n" << std::endl;
		}
	}

	if(nResult == ACS_CC_SUCCESS)
	{
		//std::cout << " --> Param value########: " << *(int*)Param.attrValues[0] << std::endl;

		(*iVal) = (*(int*)Param.attrValues[0]);

		nResult = omHandler.Finalize();

		if (nResult != ACS_CC_SUCCESS)
		{
			//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
		}
	}

  return (nResult == ACS_CC_SUCCESS);
}

bool XCOUNTLS_Util::stringToUlongMagazine (std::string value, unsigned long &uvalue)
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
