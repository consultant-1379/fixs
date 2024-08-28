
#include "FIXS_CCH_Util.h"
//#include "ACS_TRA_Trace.h"
#include "ACS_CS_API.h"
//ACS_TRA_trace traceUtil = ACS_TRA_DEF("utility", "C512");

static bool isDebugEnabled = false;

bool CCH_Util::isDebug()
{
	return isDebugEnabled;
}

void CCH_Util::setDebug(bool isDebug)
{
	isDebugEnabled = isDebug;
}

bool CCH_Util::stringToUlongMagazine (std::string value, unsigned long &uvalue)
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

//bool CCH_Util::ulongToStringMagazine (unsigned long uvalue, std::string &value)
bool CCH_Util::ulongToStringMagazine (uint32_t uvalue, std::string &value)
{

	char tempValueStr[16] = {0}; //size 16, "255.255.255.255" + null character
	uint32_t tempValue;
	tempValue =  ntohl(uvalue);  //changes the representation to little-endian
	snprintf(tempValueStr, sizeof(tempValueStr) - 1, "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >> 16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
    value = tempValueStr;

	return true;

}

bool CCH_Util::stringToUshort (std::string value, unsigned short &uvalue)
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

	if ((uvalue > 28) || (uvalue == 27))
	{
		return false; //Usage
	}

	return true;
}

bool CCH_Util::ushortToString (unsigned short uvalue, std::string &value)
{

	char tmp[16] = {0};
	snprintf(tmp, sizeof(tmp) - 1, "%u", uvalue);
	value = tmp;

	return true;
}
bool CCH_Util::longlongToString (LONGLONG uvalue, std::string &value)
{
	std::stringstream ss;
	ss << uvalue;
	ss >> value;

	return true;
}

bool CCH_Util::doubleToString (double dvalue, std::string &value)
{

	std::ostringstream sstream;
	sstream << dvalue;
	value = sstream.str();

	return true;
}

bool CCH_Util::longToString (long uvalue, std::string &value)
{

	char tmp[16] = {0};
	snprintf(tmp,  sizeof(tmp) - 1,"%ld", uvalue);
	value = tmp;

	return true;
}
bool CCH_Util::stringToLong (std::string value, long &uvalue)
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

bool CCH_Util::stringToInt (std::string value, int &ivalue)
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

std::string CCH_Util::intToString (int ivalue)
{
	std::stringstream ss_value("");
	ss_value << ivalue; //try to convert operand to value number
	return ss_value.str();
}

bool CCH_Util::intToUshort (int value, unsigned short &ivalue)
{
	std::ostringstream oss;
	oss << value;

	std::string res = oss.str();

	if (stringToUshort(res, ivalue))return true;
	else return false;
}

bool CCH_Util::gprStringToHex (std::string value, int &hexvalue)
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

unsigned short CCH_Util::numOfChar (std::istringstream &istr, char searchChar)
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

void CCH_Util::reverseDottedDecStr (std::istringstream &istr)
{

	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    //      of the stream
	std::istringstream tempIstr;
	std::string tempString = "";
	int c;

	do
	{
		c = istr.get();      //get char from istringstream

		if (c == '.' || c == EOF)
		{
			(void)tempIstr.seekg(0);

			if (c == '.')
				tempIstr.str((char)c + tempString + tempIstr.str());
			else
				tempIstr.str(tempString + tempIstr.str());

			tempString = "";
		}
		else
			tempString = tempString + (char)c;
	} while (c != EOF);

	istr.str(tempIstr.str());

	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    // of the stream

}

bool CCH_Util::dottedDecToLong (std::istringstream &istr, unsigned long &value, unsigned long  lowerLimit1, unsigned long  upperLimit1, unsigned long  lowerLimit2, unsigned long  upperLimit2, unsigned long  lowerLimit3, unsigned long  upperLimit3, unsigned long  lowerLimit4, unsigned long  upperLimit4)
{
	istr.clear();     // return to the beginning
	(void)istr.seekg(0);    //      of the stream
	int c;
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
			strAddr = strAddr + (char)c;
	} while (c != EOF);

	return true;
}

bool CCH_Util::checkFigures (std::istringstream &istr)
{
	istr.clear();			// return to the beginning
	(void)istr.seekg(0);	// of the stream
	int c = istr.get();	// get first char in the istringstream

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

std::string CCH_Util::getSwSectionName(unsigned long umagazine, unsigned short uslot)
{
	std::string section;

	std::string magazine;
	std::string slot;
	if (CCH_Util::ulongToStringMagazine (umagazine, magazine))
	{
		if (ushortToString (uslot, slot))
		{
			section.append("SW:");
			section.append(magazine);
			section.append(":");
			section.append(slot);
		}
	}
	return section;
}

std::string CCH_Util::getPFMSectionName(unsigned long umagazine, unsigned short uslot)
{
	std::string section;

	std::string magazine;
	std::string slot;
	if (CCH_Util::ulongToStringMagazine (umagazine, magazine))
	{
		if (ushortToString (uslot, slot))
		{
			section.append("PFM:");
			section.append(magazine);
			section.append(":");
			section.append(slot);
		}
	}

	return section;
}

std::string CCH_Util::getIpmiSectionName(unsigned long umagazine, unsigned short uslot)
{

	std::string section;

	std::string magazine;
	std::string slot;
	if (CCH_Util::ulongToStringMagazine (umagazine, magazine))
	{
		if (ushortToString (uslot, slot))
		{
			section.append("IPMI:");
			section.append(magazine);
			section.append(":");
			section.append(slot);
		}
	}

	return section;
}

std::string CCH_Util::getPcSectionName(unsigned long umagazine, unsigned short uslot)
{
	std::string section;

	std::string magazine;
	std::string slot;
	if (CCH_Util::ulongToStringMagazine (umagazine, magazine))
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
bool CCH_Util::stringToUpper (std::string& stringToConvert)
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


bool CCH_Util::createDir (const char* dirpath, DWORD& err)
{
	UNUSED(dirpath);
	UNUSED(err);
	return false;

}

void CCH_Util::getCurrentTime(std::string& dateStr, std::string& timeStr)
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

std::string CCH_Util::getCurrentTimeInSeconds()
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

	CCH_Util::longToString(timeinseconds,secInStr);

	return secInStr;
}
void CCH_Util::getResultReason(std::string& resultValue, std::string& reasonValue, int code)
{
    reasonValue = "-";
	resultValue = "-";
	switch(code)
	{
		// SCX CODES
		case SW_ERR_OK:
			resultValue = "OK";
			reasonValue = "-";
			break;
		case SW_ERR_LOAD_RELEASE_IN_PROGRESS:
			resultValue = "NOT OK";
			reasonValue = "LOAD RELEASE IN PROGRESS";
			break;
		case SW_ERR_LOAD_RELEASE_IN_SUCCESS:
			resultValue = "OK";
			reasonValue = "-";
			break;
		case SW_ERR_LOAD_RELEASE_HOST_NOT_FOUND:
			resultValue = "NOT OK";
			reasonValue = "LOAD RELEASE HOST NOT FOUND";
			break;
		case SW_ERR_LOAD_RELEASE_LOGIN_INCORRECT:
			resultValue = "NOT OK";
			reasonValue = "LOAD RELEASE LOGIN INCORRECT";
			break;
		case SW_ERR_LOAD_RELEASE_NO_SUCH_DIRECTORY:
			resultValue = "NOT OK";
			reasonValue = "LOAD RELEASE NO SUCH DIRECTORY";
			break;
		case SW_ERR_LOAD_RELEASE_NO_SUCH_FILE:
			resultValue = "NOT OK";
			reasonValue = "LOAD RELEASE NO SUCH FILE";
			break;
		case SW_ERR_LOAD_RELEASE_ERRONEOUS_URL:
			resultValue = "NOT OK";
			reasonValue = "LOAD RELEASE ERRONEOUS URL";
			break;
		case SW_ERR_LOAD_RELEASE_ERROR:
			resultValue = "NOT OK";
			reasonValue = "LOAD RELEASE ERROR";
			break;
		case SW_ERR_REMOVE_RELEASE_IN_PROGRESS:
			resultValue = "NOT OK";
			reasonValue = "REMOVE RELEASE IN PROGRESS";
			break;
		case SW_ERR_REMOVE_RELEASE_SUCCESS:
			resultValue = "NOT OK";
			reasonValue = "REMOVE RELEASE SUCCESS";
			break;
		case SW_ERR_REMOVE_RELEASE_ERROR:
			resultValue = "NOT OK";
			reasonValue = "REMOVE RELEASE ERROR";
			break;
		case SW_ERR_STATUS_UNINITIATED:
			resultValue = "NOT OK";
			reasonValue = "STATUS UNINITIATED";
			break;
		case SW_ERR_CANCELLED:
			resultValue = "NOT OK";
			reasonValue = "MANUALLY CANCELLED";
			break;
		case SW_ERR_OLD_SOFTWARE_STILL_ACTIVE:
			resultValue = "NOT OK";
			reasonValue = "OLD SOFTWARE IS STILL ACTIVE";
			break;
		case SW_ERR_COMMIT_ERROR:
			resultValue = "NOT OK";
			reasonValue = "COMMIT FAILED";
			break;
		case SW_ERR_AUTO_FAILURE:
			resultValue = "NOT OK";
			reasonValue = "AUTOMATIC FAILURE";
			break;
		case SW_ERR_MANUAL_FAILURE:
			resultValue = "NOT OK";
			reasonValue = "MANUAL FAILURE";
			break;

		// IPT CODES
		case LM_ERR_OK:
			resultValue = "OK";
			reasonValue = "-";
			break;

		case LM_ERR_MANUAL_ROLLBACK_SUCCESS:
			resultValue = "NOT OK";
			reasonValue = "MANUALLY CANCELLED";
			break;

		case LM_ERR_CANCEL:
			resultValue = "NOT OK";
			reasonValue = "MANUALLY CANCELLED";
			break;
		case LM_ERR_COMMIT_ERROR:
			resultValue = "NOT OK";
			reasonValue = "COMMIT ERROR";
			break;

		case LM_ERR_ACTIVATE_ERROR:
			resultValue = "NOT OK";
			reasonValue = "ERROR ACTIVATING LOAD MODULE";
			break;
		case LM_ERR_MANUAL_ROLLBACK_FAILURE:
			resultValue = "NOT OK";
			reasonValue = "MANUAL ROLLBACK FAILED";
			break;
		case LM_ERR_AUTOMATIC_ROLLBACK_SUCCESS:
			resultValue = "NOT OK";
			reasonValue = "AUTOMATIC ROLLBACK SUCCESS";
			break;
		case LM_ERR_AUTOMATIC_ROLLBACK_FAILURE:
			resultValue = "NOT OK";
			reasonValue = "AUTOMATIC ROLLBACK FAILED";
			break;
		default:
			resultValue = "-";
			reasonValue = "-";
			break;
	}

	//std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" LAST RESULT:"<< resultValue <<" LAST REASON:"<< reasonValue <<std::endl;
}

void CCH_Util::getResultReason(int& resultValue, int& reasonValue, int code)
{
    reasonValue = SW_RP_EXECUTED;
	resultValue = SW_RP_OK;
	switch(code)
	{
		// SCX CODES
		case SW_ERR_OK:
		case SW_ERR_LOAD_RELEASE_IN_SUCCESS:
			resultValue = SW_RP_OK;
			reasonValue = SW_RP_EXECUTED;
			break;

		case SW_ERR_AUTO_FAILURE:
			resultValue = SW_RP_NOT_OK;
			//reasonValue = SW_RP_ACTIVATE_FAILURE;
                        reasonValue   = SW_RP_ROLLBACK_FAILURE;
			break;

		case SW_ERR_CANCELLED:
			resultValue = SW_RP_NOT_OK;
			reasonValue = SW_RP_MANUALLY_CANCELLED;
			break;

		case SW_ERR_LOAD_RELEASE_IN_PROGRESS:
			resultValue = SW_RP_OK;
			reasonValue = SW_RP_EXECUTED;//			reasonValue = "LOAD RELEASE IN PROGRESS";
			break;

		case SW_ERR_LOAD_RELEASE_ERROR:
		case SW_ERR_REMOVE_RELEASE_ERROR:
			resultValue = SW_RP_NOT_OK;
			reasonValue = SW_RP_PREPARE_FAILURE;
			break;

		case SW_ERR_COMMIT_ERROR:
			resultValue = SW_RP_NOT_OK;
			reasonValue = SW_RP_COMMIT_FAILURE;
			break;

		case SW_ERR_MANUAL_FAILURE:
			resultValue = SW_RP_NOT_OK;
			reasonValue = SW_RP_CANCEL_FAILURE;
			break;

		default:
			resultValue = SW_RP_OK;
			reasonValue = SW_RP_EXECUTED;
			break;
	}

}


std::string CCH_Util::replaceSubstr( const std::string& content, const std::string& instance, const std::string& replacement )
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

//CCH_Util::EnvironmentType CCH_Util::getEnvironment(void)
//{
//
//	CCH_Util::EnvironmentType environment;
//	IMM_Interface::getEnvironment(environment);
//
//	return environment;
//
//}

std::string CCH_Util::getEnvironmentType(CCH_Util::EnvironmentType env)
{
	std::string envStr;
	switch(env)
	{
		case CCH_Util::SINGLECP_NOTCBA:
			envStr = "SINGLECP_NOTCBA";
			break;
		case CCH_Util::SINGLECP_CBA:
			envStr = "SINGLECP_CBA";
			break;
		case CCH_Util::SINGLECP_DMX:
			envStr = "SINGLECP_DMX";
			break;
		case CCH_Util::MULTIPLECP_NOTCBA:
			envStr = "MULTIPLECP_NOTCBA";
			break;
		case CCH_Util::MULTIPLECP_CBA:
			envStr = "MULTIPLECP_CBA";
			break;
		case CCH_Util::MULTIPLECP_DMX:
			envStr = "MULTIPLECP_DMX";
			break;
		default:
			envStr = "UNKNOWN";
	}
	return envStr;
}


CCH_Util::APGtype CCH_Util::getAPGtype(void)
{
        CCH_Util::APGtype type;

        ACS_CS_API_CommonBasedArchitecture::ApgNumber aptype;
        
        if (ACS_CS_API_NetworkElement::getApgNumber(aptype) == ACS_CS_API_NS::Result_Success)
        {
	      switch(aptype)
	      {
          	case ACS_CS_API_CommonBasedArchitecture::AP1:
		type = AP1;
         	break;
	
		case ACS_CS_API_CommonBasedArchitecture::AP2:
		type = AP2;
	 	break;

	  	default:
		type = Unknown;
		break;
	      } 
	 }
	else 
	      {
		type = Unknown;
              }

	return type;
}  	

CCH_Util::EnvironmentType CCH_Util::isRightIptEnvironment(void)
{
	bool isCBA = false;
	bool multipleCPSystem = false;

	UNUSED(isCBA);
	UNUSED(multipleCPSystem);

	return UNKNOWN;
}

//--------------------------------------------------------------
//	Convert multibyte character string to wide character string
//--------------------------------------------------------------
std::wstring CCH_Util::mbsToWcs(const std::string& str)
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

//-----------------------
//	Create symbolic link
// An alternative way - system ("junction -s <link folder> <dir>");
//-----------------------
int CCH_Util::createSymbolicLink(const std::string& link, const std::string& dir)
{
	UNUSED(link);
	UNUSED(dir);
	return 0;
}

std::string CCH_Util::ulongToStringIP (unsigned long ipaddress)
{
	char tempValueStr[16] = {0};
	uint32_t tempValue = ipaddress;
	snprintf(tempValueStr, sizeof(tempValueStr), "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >> 16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
    std::string IP = tempValueStr;
	return IP;
}

int CCH_Util::getMeaningGPRData (std::string value)
{
	int gprValue = GPR_Unknown;

	if (!gprStringToHex(value,gprValue))
	{
		std::cout << "ERROR CONVERTING CODE TO INT!" << std::endl;
		return gprValue;
	}


	//std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data Code: "<< std::hex << gprValue <<std::endl;

	switch (gprValue)
	{
	case 0x11:
		gprValue = GPR_Fetching_TFTP_server_address;
		break;

	case 0x21:
		gprValue = GPR_Checking_access_of_dhcpinfo;
		break;

	case 0x22:
		gprValue = GPR_Fetching_DHCP_information;
		break;

	case 0x23:
		gprValue = GPR_Checking_Bootfile_Name;
		break;

	case 0x24:
		gprValue = GPR_Checking_NTP_Server;
		break;

	case 0x31:
		gprValue = GPR_Downloading_tar_file;
		break;

	case 0x41:
		gprValue = GPR_Extracting_tar_file;
		break;

	case 0x42:
		gprValue = GPR_Check_after_extracting_tar_file;
		break;

	case 0x51:
		gprValue = GPR_Create_snapshot_of_existing_RPM;
		break;

	case 0x61:
		gprValue = GPR_Starting_uninstall_RPM;
		break;

	case 0x62:
		gprValue = GPR_Restoring_snapshot_after_failed_uninstall;
		break;

	case 0x63:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0x64:
		gprValue = GPR_Starting_activating_after_uninstall;
		break;

	case 0x65:
		gprValue = GPR_Restoring_snapshot_after_failed_activation;
		break;

	case 0x66:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0x71:
		gprValue = GPR_Starting_install_RPM_configuration;
		break;

	case 0x72:
		gprValue = GPR_Restoring_snapshot_after_failed_install;
		break;

	case 0x73:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0x7d:
		gprValue = GPR_Downloading_OS_RPM;
		break;

	case 0x7e:
		gprValue = GPR_Restoring_snapshot_after_failed_download_OS_RPM;
		break;

	case 0x7f:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0x81:
		gprValue = GPR_Upgrading_OS_RPM;
		break;

	case 0x82:
		gprValue = GPR_Restoring_snapshot_after_failed_upgrade_OS_RPM;
		break;

	case 0x83:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0x91:
		gprValue = GPR_Starting_activating_after_install;
		break;

	case 0x92:
		gprValue = GPR_Restoring_snapshot_after_failed_activation;
		break;

	case 0x93:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0xa1:
		gprValue = GPR_Initiating_reboot_after_OS_RPM_upgrade;
		break;

	case 0xff:
		gprValue = GPR_Successful_upgrade_IPT;
		break;

	default:
		std::cout<<"ERROR: GPR DATA VALUE NOT RECOGNIZED"<<std::endl;
		gprValue = GPR_Unknown;
	}

	return gprValue;
}

std::string CCH_Util::traslateGPRDataValue (int code)
{
	std::string codeString="";

	std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data Int Code: "<< code <<std::endl;

	switch (code)
	{
	case GPR_Fetching_TFTP_server_address:
		codeString="Fetching_TFTP_server_address";
		break;

	case GPR_Checking_access_of_dhcpinfo:
		codeString="Checking_access_of_dhcpinfo";
		break;

	case GPR_Fetching_DHCP_information:
		codeString="Fetching_DHCP_information";
		break;

	case GPR_Checking_Bootfile_Name:
		codeString="Checking_Bootfile_Name";
		break;

	case GPR_Checking_NTP_Server:
		codeString="Checking_NTP_Server";
		break;

	case GPR_Downloading_tar_file:
		codeString="Downloading_tar_file";
		break;

	case GPR_Extracting_tar_file:
		codeString="Extracting_tar_file";
		break;

	case GPR_Check_after_extracting_tar_file:
		codeString="Check_after_extracting_tar_file";
		break;

	case GPR_Create_snapshot_of_existing_RPM:
		codeString="Create_snapshot_of_existing_RPM";
		break;

	case GPR_Starting_uninstall_RPM:
		codeString="Starting_uninstall_RPM";
		break;

	case GPR_Restoring_snapshot_after_failed_uninstall:
		codeString="Restoring_snapshot_after_failed_uninstall";
		break;

	case GPR_Starting_sync_after_restoring_snapshot:
		codeString="Starting_sync_after_restoring_snapshot";
		break;

	case GPR_Starting_activating_after_uninstall:
		codeString="Starting_activating_after_uninstall";
		break;

	case GPR_Restoring_snapshot_after_failed_activation:
		codeString="Restoring_snapshot_after_failed_activation";
		break;

	case GPR_Starting_install_RPM_configuration:
		codeString="Starting_install_RPM_configuration";
		break;

	case GPR_Restoring_snapshot_after_failed_install:
		codeString="Restoring_snapshot_after_failed_install";
		break;

	case GPR_Downloading_OS_RPM:
		codeString="Downloading_OS_RPM";
		break;

	case GPR_Restoring_snapshot_after_failed_download_OS_RPM:
		codeString="Restoring_snapshot_after_failed_download_OS_RPM";
		break;

	case GPR_Upgrading_OS_RPM:
		codeString="Upgrading_OS_RPM";
		break;

	case GPR_Restoring_snapshot_after_failed_upgrade_OS_RPM:
		codeString="Restoring_snapshot_after_failed_upgrade_OS_RPM";
		break;

	case GPR_Starting_activating_after_install:
		codeString="Starting_activating_after_install";
		break;

	case GPR_Initiating_reboot_after_OS_RPM_upgrade:
		codeString="Initiating_reboot_after_OS_RPM_upgrade";
		break;

	case GPR_Successful_upgrade_IPT:
		codeString="Successful_upgrade_IPT";
		break;

	default:
		codeString="Unknown";
		std::cout<<"ERROR: GPR DATA VALUE NOT RECOGNIZED"<<std::endl;
	}

	return codeString;
}

bool CCH_Util::isFlashValue (std::string value)
{
	if (value.size() < 5) return false;
	char code = value[5];

	if (code == '0')
		return true;

	return false;
}

bool CCH_Util::isNetworkValue (std::string value)
{
	if (value.size() < 5) return false;
	char code = value[5];

	if (code == '1')
		return true;

	return false;
}

std::string CCH_Util::GetAPHostName()
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

bool CCH_Util::stringTolonglong (std::string value, LONGLONG &ivalue)
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

LONGLONG CCH_Util::getEpochTimeInSeconds()
{
	LONGLONG timeinseconds;
	std::string secInStr;

	time_t sec;
	sec = time(NULL);  // Get the current time in epch sec

	timeinseconds = static_cast<LONGLONG> (sec);

	return timeinseconds;
}

// calculate number of digit present till decimal point
int CCH_Util::calculateDigit(double x)
{
	int count =0;
	while(x > 1)
	{
		x = x/10;
		count++;
	}
	return count;
}

bool CCH_Util::gprScxStringToHex (std::string value, int &hexvalue)
{

	std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data String Value: "<< value.c_str() <<std::endl;
	if (value.size() == 24)
	{
		std::string code = value.substr(21,22);
		std::istringstream istr(code);

		if (istr >> std::hex >> hexvalue) //try to convert operand to value number
		{
		//	std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data Hex Value: "<< std::hex<< hexvalue<<std::endl;
			return true;
		}
	}

	return false;
}


int CCH_Util::getMeaningScxGPRData (std::string value)
{
	int gprValue = GPR_Unknown;

	if (!gprScxStringToHex(value,gprValue))
	{
		std::cout << "ERROR CONVERTING CODE TO INT!" << std::endl;
		return gprValue;
	}


//	std::cout<< "DBG: "<<__FUNCTION__<<"@"<<__LINE__<< " GPR Data Code: "<< std::hex << gprValue <<std::endl;

	switch (gprValue)
	{
	case 0x11:
		gprValue = GPR_Fetching_TFTP_server_address;
		break;

	case 0x21:
		gprValue = GPR_Checking_access_of_dhcpinfo;
		break;

	case 0x22:
		gprValue = GPR_Fetching_DHCP_information;
		break;

	case 0x23:
		gprValue = GPR_Checking_Bootfile_Name;
		break;

	case 0x24:
		gprValue = GPR_Checking_NTP_Server;
		break;

	case 0x31:
		gprValue = GPR_Downloading_tar_file;
		break;

	case 0x41:
		gprValue = GPR_Extracting_tar_file;
		break;

	case 0x42:
		gprValue = GPR_Check_after_extracting_tar_file;
		break;

	case 0x51:
		gprValue = GPR_Create_snapshot_of_existing_RPM;
		break;

	case 0x61:
		gprValue = GPR_Starting_uninstall_RPM;
		break;

	case 0x62:
		gprValue = GPR_Restoring_snapshot_after_failed_uninstall;
		break;

	case 0x63:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0x64:
		gprValue = GPR_Starting_activating_after_uninstall;
		break;

	case 0x65:
		gprValue = GPR_Restoring_snapshot_after_failed_activation;
		break;

	case 0x66:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0x71:
		gprValue = GPR_Starting_install_RPM_configuration;
		break;

	case 0x72:
		gprValue = GPR_Restoring_snapshot_after_failed_install;
		break;

	case 0x73:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0x7d:
		gprValue = GPR_Downloading_OS_RPM;
		break;

	case 0x7e:
		gprValue = GPR_Restoring_snapshot_after_failed_download_OS_RPM;
		break;

	case 0x7f:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0x81:
		gprValue = GPR_Upgrading_OS_RPM;
		break;

	case 0x82:
		gprValue = GPR_Restoring_snapshot_after_failed_upgrade_OS_RPM;
		break;

	case 0x83:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0x91:
		gprValue = GPR_Starting_activating_after_install;
		break;

	case 0x92:
		gprValue = GPR_Restoring_snapshot_after_failed_activation;
		break;

	case 0x93:
		gprValue = GPR_Starting_sync_after_restoring_snapshot;
		break;

	case 0xa1:
		gprValue = GPR_Initiating_reboot_after_OS_RPM_upgrade;
		break;

	case 0xff:
		gprValue = GPR_Successful_upgrade_IPT;
		break;

	default:
		std::cout<<"ERROR: GPR DATA VALUE NOT RECOGNIZED"<<std::endl;
		gprValue = GPR_Unknown;
	}

	return gprValue;
}

bool CCH_Util::CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists, long int size)
{
	if ( bFailedIfExists && CCH_Util::PathFileExists(szDestFile) )
	{
		std::cout << "Dest file exists : " << szDestFile << std::endl;
		return false;
	}

	FILE* pSourceFile;
	FILE* pDestFile;
	unsigned char copy_buffer[size];
	int byte_count_read = 0;
	int byte_count_written = 0;

	pSourceFile = ACE_OS::fopen(szSourceFile, "rb");
	if ( !pSourceFile )
	{       std::cout << "errorno7: " << errno << std::endl;   
	       	std::cout << "could not open source file : " << szSourceFile << std::endl;
		return false;
	}

	pDestFile = ACE_OS::fopen(szDestFile, "wb");
	if ( !pDestFile )
	{
		std::cout << "errorno6: " << errno << std::endl;
		std::cout << "could not open dest file : " << szDestFile << std::endl;
		ACE_OS::fclose(pSourceFile);
		return false;
	}

	std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " File created:" << szDestFile <<  std::endl;

	for(;;)
	{
		byte_count_read = ACE_OS::fread(copy_buffer, 1, size, pSourceFile);
		if ( byte_count_read > 0 )
		{
			byte_count_written = ACE_OS::fwrite(copy_buffer, 1, byte_count_read, pDestFile);

			if ( byte_count_written != byte_count_read )
			{
                                std::cout << "errorno8: " << errno << std::endl;
				ACE_OS::fclose(pSourceFile);
				ACE_OS::fclose(pDestFile);
				std::cout << "byte_count_written not equal to byte_count_read" << std::endl;
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




bool CCH_Util::CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists)
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

   unsigned char *copy_buffer;

   copy_buffer=(unsigned char*)malloc(sizeof(unsigned char)*(size+1));

   if(!copy_buffer)
	   return false;

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


bool CCH_Util::PathFileExists(const char* file)
{
	struct stat st;
		//check if exist
		if(stat(file,&st) != 0)	return false;
		else return true;

}


bool CCH_Util::isSymbolicLinkExists(const char* file)
{
    struct stat st;
   
       
       if(stat(file,&st) != 0){ 
         return false;
       }
       else{
             if(S_ISLNK(st.st_mode) != 0) return false;
             else return true;
       }
   return false;
}  


int CCH_Util::findFile(char* path, char* type, std::vector<std::string> &result)
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




int CCH_Util::removeFolder(const char* _fullpath){

	DIR		*dp;
	struct dirent	*dirp;
	struct stat 	statbuf;
	char		*new_fullpath;
	int 		ret;

	if (stat(_fullpath,&statbuf) != 0)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Direcoty: "<< _fullpath << " doesn't exist !!! "<< std::endl;
		return 0;
	}

	if(!S_ISDIR(statbuf.st_mode))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Is not a Direcoty: "<< _fullpath << std::endl;
		return LSTAT_ERROR;
	}


	if((dp = opendir(_fullpath)) == NULL)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Read permission is denied for the directory named: "<< _fullpath << std::endl;
		return OPENDIR_ERROR;
	}

	//opendir OK

	while((dirp = readdir(dp)) != NULL)
	{

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

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " New full path: "<< new_fullpath << std::endl;

		if(S_ISDIR(statbuf.st_mode))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "the current file is a directory it needs remove the entire content "<< std::endl;
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
		else {
			if(remove(new_fullpath) < 0)
			{
				free(new_fullpath);
				closedir(dp);
				return REMOVE_ERROR;
			}else std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Removed file: "<< new_fullpath << std::endl;
		}

		free(new_fullpath);

	}// end-while

	//close directory
	if(closedir(dp) < 0)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "error to close directory "<< std::endl;
		return CLOSEDIRR_ERROR;
	}else std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "close directory "<< std::endl;

	//remove directory
	if(remove(_fullpath) < 0)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "error to remove: "<< _fullpath << std::endl;
		return REMOVE_ERROR;

	}else std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Removed folder: "<< _fullpath << std::endl;

	return 0;

}// end-removeFolder

bool CCH_Util::CheckExtention(const char* file, const char* type_file)
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


void CCH_Util::stringIpToUlong(const char *ip, unsigned long &uvalue)
{
	unsigned long temp = inet_addr(ip);
	uvalue = htonl(temp);
}


void CCH_Util::findAndReplace( string &source, const string find, string replace )
{
    size_t j;
    for ( ; (j = source.find( find )) != string::npos ; )
    {
        source.replace( j, find.length(), replace );
    }
}


void CCH_Util::trim(string& str)
{
	string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
}

void CCH_Util::findAndRemove( string &source, const string find)
{
	size_t j;
	for ( ; (j = source.find( find )) != string::npos ; )
	{
		source.erase(j,1);
	}
}

bool CCH_Util::findMetadataFile (std::string &filenameSh, std::string folder, bool isSmx)
{
	DIR *dp;
	bool res = false;
	struct dirent *dirp;
	if((dp  = opendir(folder.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << folder.c_str() << endl;
		return false;
	}

	while ((dirp = readdir(dp)) != NULL) {
		if(!isSmx)
		{
			if(string(dirp->d_name).find(".sh") != string::npos)
			{
				filenameSh.assign(string(dirp->d_name));
				res = true;
				//cout << "found sh file : "<<filenameSh<<endl;
				break;
			}
		}
		else
		{

			if(string(dirp->d_name).find(".tar") != string::npos)
			{
				filenameSh.assign(string(dirp->d_name));
				res = true;
				//std::cout << "found tar file : "<<filenameSh<<std::endl;
				break;
			}
		}
	}
	closedir(dp);
	return res;

}

bool CCH_Util::findXmlFile (std::string &filenameXml, std::string folder)
{
	DIR *dp;
	struct dirent *dirp;
	bool result;
	if((dp  = opendir(folder.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << folder.c_str() << endl;
		//closedir(dp);
		return false;
	}

	while ((dirp = readdir(dp)) != NULL) {
		if(string(dirp->d_name).find(".xml") != string::npos)
		{
			filenameXml.assign(string(dirp->d_name));
			cout << "found xml file : "<<filenameXml<<endl;
			result = true;
			break;
		}else{
			result = false;
		}

	}
	closedir(dp);
	return result;
}


bool CCH_Util::deleteFile(const char *file)
{
	if (CCH_Util::PathFileExists(file))
	{
		if (remove(file)<0) return false;
		return true;
	}

	return false;
}

bool CCH_Util::renameFilewithMagAndSlot(std::string &filename, std::string mag, std::string slot)
{
	bool res = true;

	int iPos = filename.find_first_of(".");
	std::string strTmp("");
	std::string strExt("");
	if(iPos >=0)
	{
		// Extension is part of the file. Extract it
		strExt = filename.substr(iPos +1);
		// then remove from filename
		strTmp = filename.substr(0,iPos);
	}
	else res = false;

	filename = strTmp+"_"+mag+"_"+slot+"."+strExt;

	return res;
}

bool CCH_Util::removeWhiteChar(std::string &value)
{
	bool res = true;

	const char *validChar = "ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvxyz0123456789/\\_";
	std::string::size_type pos;

	while((pos = value.find_last_of(validChar)) != std::string::npos)
	{
		value = value.erase(pos,1);
	}

	return res;
}

void CCH_Util::getResultImportAction(respCode result, int &resultSwStruct, int &resultInfoSwStruct)
{
	resultSwStruct = 0;
	resultInfoSwStruct = 0;

	switch(result)
	{
	case EXECUTED:
		resultSwStruct = IMM_Util::SUCCESS;
		resultInfoSwStruct = IMM_Util::EXECUTED;
		break;

	case XMLERROR:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::INTERNAL_ERROR;
		break;

	case MAXIMUM_LOAD_MODULES_INSTALLED:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::MAXIMUM_LOAD_MODULES_INSTALLED;
		break;

	case PACKAGE_ALREADY_INSTALLED:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::PACKAGE_ALREADY_INSTALLED;
		break;

	case DISK_FULL:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::DISK_QUOTA_EXCEEDED;
		break;

	case PACKAGE_NOT_EXIST:
	case FAILED_IMPORT_ACTION_ALREADY_RUNNING:
	case FAILED_INVALID_FILENAME:
	case FAILED_PATH_NOT_EXIST:
	case FAILED_WRONG_EXTENSION:
	case PACKAGE_NOT_APPLICABLE:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::INVALID_PACKAGE;
		break;

	case INTERNALERR:
	case DIRECTORYERR:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::INTERNAL_ERROR;
		break;
	case FAILED_INVALID_FBN:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::INTERNAL_ERROR;
		break;
	case FAILED_INVALID_CONFIGURATION:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::INVALID_CONFIGURATION;
		break;

	case FAILED_FBN_NOT_ALLOWED:
        resultSwStruct = IMM_Util::FAILURE;
        resultInfoSwStruct = IMM_Util::FBN_NOT_ALLOWED;
        break;

	case NOT_ALLOWED_BACKUP_ONGOING:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::BACKUP_ONGOING;
	break;

	default:
		resultSwStruct = IMM_Util::SUCCESS;
		resultInfoSwStruct = IMM_Util::EXECUTED;
		break;
	}

}

void CCH_Util::getResultUpgradeAction(int result, int &resultSwStruct, int &resultInfoSwStruct)
{
	resultSwStruct = 0;
	resultInfoSwStruct = 0;
	switch(result)
	{
	case EXECUTED:
		resultSwStruct = IMM_Util::SUCCESS;
		resultInfoSwStruct = IMM_Util::ASW_EXECUTED;
		break;

	case HARDWARE_NOT_PRESENT:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::ASW_HARDWARE_NOT_PRESENT;
		break;

	case SNMP_FAILURE:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::ASW_SNMP_FAILURE;
		break;

	case SW_ALREADY_ON_BOARD://PACKAGE_ALREADY_INSTALLED:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::ASW_LM_ALREADY_PRESENT;
		break;

	case UPGRADE_ALREADY_ONGOING:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::ASW_UPGRADE_ALREADY_ONGOING;
		break;

	case WRONG_OPTION:
	case WRONG_OPTION_ACTIVATE:
	case WRONG_OPTION_CANCEL:
	case WRONG_OPTION_COMMIT:
	case WRONG_OPTION_PREPARE:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::ASW_WRONG_ACTION_IN_THIS_STATE;
		break;

	case NOT_ALLOWED_BACKUP_ONGOING:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::ASW_BACKUP_ONGOING;
		break;

	case NO_REPLY_FROM_CS:
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::ASW_NO_REPLY_FROM_CS;
		break;

	case NOT_ALLOWED_BY_CP:
		std::cout << __FUNCTION__ << "@" << __LINE__ << " +++NOT ALLOWED: "<<std::endl;
		resultSwStruct = IMM_Util::FAILURE;
		resultInfoSwStruct = IMM_Util::ASW_NOT_ALLOWED_BY_CP;
		break;
	case PACKAGE_NOT_APPLICABLE:
		std::cout << __FUNCTION__ << "@" << __LINE__ << " PACKAGE_NOT_APPLICABLE: "<<std::endl;
                resultSwStruct = IMM_Util::FAILURE;
                resultInfoSwStruct = IMM_Util::ASW_PACKAGE_NOT_APPLICABLE;
                break;
	case IRONSIDE_FAILURE:
		std::cout << __FUNCTION__ << "@" << __LINE__ << " COMMUNICATION_FAILURE: "<<std::endl;
                resultSwStruct = IMM_Util::FAILURE;
                resultInfoSwStruct = IMM_Util::ASW_IRONSIDE_FAILURE;
                break;
	case OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE:
		std::cout << __FUNCTION__ << "@" << __LINE__ << " OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE:  "<<std::endl;
		resultSwStruct = IMM_Util::FAILURE;
                resultInfoSwStruct = IMM_Util::ASW_OTHER_IPLB_IN_CLUSTER_NOT_ACTIVE;
                break;
	case BOARD_ALREADY_BLOCKED:
		std::cout << __FUNCTION__ << "@" << __LINE__ << " BOARD_ALREADY_BLOCKED :  "<<std::endl;
		resultSwStruct = IMM_Util::FAILURE;
                resultInfoSwStruct = IMM_Util::ASW_BOARD_ALREADY_BLOCKED;
                break;
	default:
		resultSwStruct = IMM_Util::SUCCESS;
		resultInfoSwStruct = IMM_Util::EXECUTED;
		break;
	}

}


bool CCH_Util::getFolderList (std::vector<std::string> &listFolder, std::string folder_parent)
{
	DIR *dp;
	struct dirent *dirp;
	struct stat 	statbuf;

	if((dp  = opendir(folder_parent.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << folder_parent.c_str() << endl;
		//closedir(dp);
		return false;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if((strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0))
		{
			continue;	//ignore dot and dot-dot
		}

		std::string new_path = folder_parent + "/" + dirp->d_name;

		stat( new_path.c_str(), &statbuf );

		if (S_ISDIR( statbuf.st_mode )){

			listFolder.push_back(new_path);
		}

	}
	closedir(dp);
	return true;
}

bool CCH_Util::CopyFolder(const char* srcFolder,const char* dstFolder)
{
	DIR *dp;
	struct dirent *dirp;
	struct stat 	statbuf;
	char* new_srcFolder;
	char* new_dstFolder;

	if (stat(srcFolder,&statbuf) != 0){

		std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Directory:" << srcFolder <<"doesn't exist!" << std::endl;
		return false;
	}

	if (!S_ISDIR(statbuf.st_mode)) {

		std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Is not a Directory:" << srcFolder <<  std::endl;
		return false;

	}
	if((dp  = opendir(srcFolder)) == NULL) {
		cout << "Error(" << errno << ") opening " << srcFolder << endl;
		//closedir(dp);
		return false;
	}

	bool res;

	while ((dirp = readdir(dp)) != NULL)
	{
		res = true;

		if((strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0))
		{
			continue;	//ignore dot and dot-dot
		}

		new_srcFolder = (char*) calloc((strlen(srcFolder) + strlen(dirp->d_name) + 2),sizeof(char));
		if(!new_srcFolder)
		{
		        std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Errorno1:" << errno <<  std::endl;
			closedir(dp);
			return false;
		}
		memset(new_srcFolder, 0, (strlen(srcFolder) + strlen(dirp->d_name) + 2));

		new_dstFolder = (char*) calloc((strlen(dstFolder) + strlen(dirp->d_name) +2),sizeof(char));
		if(!new_dstFolder)
		{
                        std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Errorno2:" << errno <<  std::endl;
			free(new_srcFolder);
			closedir(dp);
			return false;
		}
		memset(new_dstFolder, 0 , (strlen(dstFolder) + strlen(dirp->d_name) +2));

		strcat(new_srcFolder,srcFolder);
		strcat(new_srcFolder,"/");
		strcat(new_srcFolder,dirp->d_name);
		//strcat(new_srcFolder,"\0");

		strcat(new_dstFolder, dstFolder);
		strcat(new_dstFolder, "/");
		strcat(new_dstFolder,dirp->d_name);
		//strcat(new_srcFolder,"\0");

		if (stat(dstFolder,&statbuf) != 0){
			res=ACS_APGCC::create_directories(dstFolder,ACCESSPERMS);
                        if (!res){
				std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Errorno10:" << errno <<  std::endl;
				free(new_srcFolder);
				free(new_dstFolder);
				closedir(dp);
				return false;
			}
			std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Directory created:" << dstFolder <<  std::endl;
		}


		if(stat( new_srcFolder, &statbuf) != 0)
		{
		        std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Errorno3:" << errno <<  std::endl;
			free(new_srcFolder);
			free(new_dstFolder);
			closedir(dp);
			return false;
		}

		if (S_ISDIR( statbuf.st_mode )){
      
			res=ACS_APGCC::create_directories(new_dstFolder,ACCESSPERMS);
			if (!res){
				std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Errorno11:" << errno <<  std::endl;
				free(new_srcFolder);
				free(new_dstFolder);
				closedir(dp);
				return false;
			}
			std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Directory created:" << new_dstFolder <<  std::endl;
			res = CCH_Util::CopyFolder(new_srcFolder, new_dstFolder);
			if(!res){
                                std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Errorno4:" << errno <<  std::endl;
				free(new_srcFolder);
				free(new_dstFolder);
				closedir(dp);
				return false;
				}
		}
		else
		{
			//std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " Is a file:" << new_srcFolder << std::endl;
			res = CCH_Util::CopyFile(new_srcFolder,new_dstFolder,false,4096);
			if(!res){
                                std::cout <<"DBG:"<< __FUNCTION__ << "@" << __LINE__ << " Errorno5:" << errno <<  std::endl;
				free(new_srcFolder);
				free(new_dstFolder);
				closedir(dp);
				return false;
				}
		}

		free(new_srcFolder);
		free(new_dstFolder);

	} //end of while loop

	if(!res){
		free(new_srcFolder);
		free(new_dstFolder);
	}

	closedir(dp);
	return true;
}

bool CCH_Util::findFile (std::string fileName, std::string folder)
{
	bool found = false;
	DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir(folder.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << folder.c_str() << endl;
		//closedir(dp);
		return false;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if(strcmp(dirp->d_name,fileName.c_str()) == 0)
		{
			found = true;
			break;
		}
	}
	closedir(dp);
	return found;
}


void CCH_Util::readFile (std::string filePath, std::string &contentFile)
{
	int size = 0;
	struct stat info;
	stat(filePath.c_str(), &info);

	size = info.st_size;
	char *content = (char *)malloc(size * sizeof(char));
	FILE *fp = fopen(filePath.c_str(), "rb");
	fread(content, info.st_size, 1, fp);
	fclose(fp);

	contentFile = std::string(content);

	free(content);
	content=NULL;

}

void CCH_Util::getUsedMagAndSlot (std::string usedValue, int &magSlot)
{
	int magazine=0;
	int islot=0;

	std::string mag = ACS_APGCC::before(usedValue,".");
	std::string slot = ACS_APGCC::after(usedValue,"_");
	stringToInt(mag,magazine);
	stringToInt(slot,islot);
	magSlot = (magazine*100)+ islot;
}

void CCH_Util::sortUsedVector (std::vector<std::string> &usedValue)
{
	std::string tempUsedValue("");
	int magSlot_i = 0;
	int magSlot_j = 0;

	if (usedValue.size() <= 1)
		return;

	for (unsigned int i=0; i<usedValue.size()-1;i++)
	{
		magSlot_i = 0;

		getUsedMagAndSlot(usedValue[i],magSlot_i);

		for (unsigned int j=i; j<usedValue.size();j++)
		{
			magSlot_j = 0;
			getUsedMagAndSlot(usedValue[j],magSlot_j);

			if (magSlot_j < magSlot_i)
			{
				tempUsedValue = usedValue[i];
				usedValue[i] = usedValue[j];
				usedValue[j] = tempUsedValue;
			}

		}
	}
}
bool CCH_Util::createFile(std::string complete_path)
{
	bool res = true;

	ofstream write (complete_path.c_str());

	if (write.fail())
	{
		cout << "Failed to create file!\n";
		res = false;
	}

	write.close();

	return res;
}

 int CCH_Util::isBoardPresent (std::string magazine, std::string slot)
{
	unsigned long umagazine;
	unsigned short uslot;

	CCH_Util::stringToUlongMagazine(magazine,umagazine);
	CCH_Util::stringToUshort(slot,uslot);
	int retValue = -1;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
		{
			ACS_CS_API_IdList boardList;
			boardSearch->setMagazine(umagazine);
			boardSearch->setSlot(uslot);

			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{

				if (boardList.size() == 0)
				{
					// trace: no board found. The board related hasn't been configured yet.
					retValue = 0;
				}
				else
				{
					//boardId = boardList[0];
					retValue = 1;
				}
			}
			else
			{
				// GetBoardIds failed, error from CS
				retValue = 0;
			}
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			retValue = 0;
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
		retValue = 0;
	}
	return retValue;

}

bool CCH_Util::findRfuDataFile (std::string &filenametz, std::string folder)
{
	DIR *dp;
	bool res = false;
	struct dirent *dirp;
	if((dp  = opendir(folder.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << folder.c_str() << endl;
		return false;
	}

	while ((dirp = readdir(dp)) != NULL) {
		if(string(dirp->d_name).find(".tar.gz") != string::npos)
		{
			filenametz.assign(string(dirp->d_name));
			res = true;
			cout << "found tar.gz file : " << filenametz.c_str() <<endl;
			break;
		}
	}
	closedir(dp);
	return res;
}


bool CCH_Util::findJsonFile (std::string &filenameJson, std::string folder)
{
	DIR *dp;
	bool res = false;
	struct dirent *dirp;
	if((dp  = opendir(folder.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << folder.c_str() << endl;
		return false;
	}

	while ((dirp = readdir(dp)) != NULL) {
		if(string(dirp->d_name).find(".json") != string::npos)
		{
			filenameJson.assign(string(dirp->d_name));
			res = true;
			cout << "found tar.gz file : " << filenameJson.c_str() <<endl;
			break;
		}
	}
	closedir(dp);
	return res;
}

int CCH_Util::getCmxSoftwareVersion (unsigned long umagazine,unsigned short uslot, int & sw_version)
{
	
	int retValue = EXECUTED;
	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (hwc)
	{
		ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
		if (boardSearch)
                {
			boardSearch->setMagazine(umagazine);
			boardSearch->setSlot(uslot);
			ACS_CS_API_IdList boardList;
			ACS_CS_API_NS::CS_API_Result returnValue = hwc->getBoardIds(boardList, boardSearch);
			if (returnValue == ACS_CS_API_NS::Result_Success)
			{
				if (boardList.size() == 1)
				{
					BoardID boardId = boardList[0];
					unsigned short fbn = 0;
					returnValue = hwc->getFBN(fbn, boardId);
					if (returnValue == ACS_CS_API_NS::Result_Success)
					{
						if(fbn == ACS_CS_API_HWC_NS::FBN_CMXB)
						{
							uint16_t version = -1;
							ACS_CS_API_NS::CS_API_Result returnValue_sw = hwc->getSoftwareVersionType(version,boardId);
							if (returnValue_sw == ACS_CS_API_NS::Result_Success)
							{
								sw_version = version;
								std::cout << "\n sw_version = "<< sw_version <<std::endl;
								retValue = EXECUTED;
							}
							else
							{
								std::cout << "[line:" << __LINE__ << "] Serious fault in CS: No software version found for cmxb"<< std::endl;
								retValue = NO_REPLY_FROM_CS;//NO_REPLY_FROM_CS
							}
						}
						else
							retValue = FUNC_BOARD_NOTALLOWED;
					}
					else
					{
						std::cout << "[" << __FUNCTION__ << "@" << __LINE__ << "] Cannot retrieve FBN from CS"<< std::endl;
						retValue = NO_REPLY_FROM_CS;
					}
				}
				else
				{
					std::cout << "[" << __FUNCTION__ << "@" << __LINE__ << "] Board List is empty" << std::endl;
					retValue = HARDWARE_NOT_PRESENT;
				}
			}
			else if ((returnValue == ACS_CS_API_NS::Result_NoValue) || (returnValue == ACS_CS_API_NS::Result_NoEntry))
			{
				retValue = HARDWARE_NOT_PRESENT;
			}
			else
			{
				std::cout << "[" << __FUNCTION__ << "@" << __LINE__ << "] GetBoardIds failed, error from CS. result:" << returnValue << std::endl;
				retValue = NO_REPLY_FROM_CS;
			}
			ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
		}
		else
		{
			retValue = NO_REPLY_FROM_CS;
		}
		ACS_CS_API::deleteHWCInstance(hwc);
	}
	else
	{
		retValue = NO_REPLY_FROM_CS;
	}

	return retValue;
}

