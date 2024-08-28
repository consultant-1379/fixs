/*
 * fixs_cch_pcDataStorage.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: xlucdor
 */

#include "FIXS_CCH_PCDataStorage.h"
//#include "xcountls_Util.h"


using namespace std;
using namespace boost::property_tree;

//******************************************************************
//
//Store sequence:
//- saveBaselineHeading()
//- for each board collected
//   saveBoardCounters (unsigned long magazine, unsigned short slot, XCOUNTLS_Util::PCData data);
//- saveBaseline();
//- commitChanges();

//Read sequence:
//- loadBaseline();
//- readBaselineHeading(std::string &currentTime);
//- for each board found in CS
//		readBoardCounters (unsigned long magazine, unsigned short slot, XCOUNTLS_Util::PCData &data);
//
//******************************************************************



namespace {
   //CRITICAL_SECTION s_cs;

   const std::string CFG_FILE_NAME("SCX_baseline.cfg");
   const std::string TMP_FILE_NAME("SCX_baseline.tmp");
   const int  MAX_LENGTH = 256;                      // Max length of buffer (for C-style string)

   //ACS_TRA_trace traceObj = ACS_TRA_DEF("FIXS_CCH_PCDataStorage", "C512");

}
/*
namespace {
	struct Lock {
		Lock() { EnterCriticalSection(&s_cs); };
		~Lock() { LeaveCriticalSection(&s_cs); };
	};
}

namespace {
	struct CritSecInit {
		CritSecInit() { InitializeCriticalSection(&s_cs); };
   } dummyObj;
}
*/


FIXS_CCH_PCDataStorage* FIXS_CCH_PCDataStorage::s_instance = 0;


std::string FIXS_CCH_PCDataStorage::m_cfgFileName = "";

std::string FIXS_CCH_PCDataStorage::m_tmpCFGFileName = "";

std::string FIXS_CCH_PCDataStorage::m_cfgPath = "";

FIXS_CCH_PCDataStorage::FIXS_CCH_PCDataStorage() {

	//not used
	//(void)getPath();       // Only data disk path can be read

	//it's needed to define a path.
	//I'll use  /data/apz/logs

	//check AP1/AP2

	m_cfgPath = "/data/apz/data/cch";
	createFolder(m_cfgPath);
	m_cfgFileName = m_cfgPath + '/';
	m_tmpCFGFileName = m_cfgFileName;
	m_cfgFileName += CFG_FILE_NAME;
	m_tmpCFGFileName += TMP_FILE_NAME;

}


FIXS_CCH_PCDataStorage * FIXS_CCH_PCDataStorage::getInstance ()
{
	//Lock lock;
	if (s_instance == 0)
	{
		s_instance = new (std::nothrow) FIXS_CCH_PCDataStorage();

		/*
		m_cfgPath = "/data/apz/logs/cch";
		s_instance->createFolder(m_cfgPath);
		m_cfgFileName = m_cfgPath + '/';
		m_tmpCFGFileName = m_cfgFileName;
		m_cfgFileName += CFG_FILE_NAME;
		m_tmpCFGFileName += TMP_FILE_NAME;
		*/
	}
	return s_instance;
}

bool FIXS_CCH_PCDataStorage::createFolder (std::string dirName){

	if (::chdir(dirName.c_str())) { //ERROR

		if (::mkdir(dirName.c_str(),  0777 ) ) {
			//std::cout << "Creation failed res: " << resMk << std::endl;
			return false;
		} else {
			//std::cout << "Dir successfully created " << resMk << std::endl;

			if (::chmod(dirName.c_str(), 0777 ) ) {
				//std::cout << "resChmod " << resChmod << std::endl;
				return false;
			} else {
				//std::cout << "Chmod ok" << resChmod << std::endl;
			}
		}
	} else {
		//std::cout << "Already presents: " << resCh << std::endl;

		if (::chmod(dirName.c_str(), 0777 ) ) {
			//std::cout << "resChmod " << resChmod << std::endl;
			return false;
		} else {
			//std::cout << "Chmod ok" << resChmod << std::endl;
		}
	}
	return true;
}




int FIXS_CCH_PCDataStorage::saveBoardCounters (unsigned long magazine, unsigned short slot, XCOUNTLS_Util::PCData data, int no_of_intrface, bool isSMXFlag)
{
	//Lock lock;
	ptree cc;

	int retVal = EXECUTED;

	std::string upperSectionName = XCOUNTLS_Util::getPcSectionName(magazine, slot);

	if(isSMXFlag)
		upperSectionName = upperSectionName +"_T";
	else
		upperSectionName = upperSectionName +"_C";

	{
		ptree &child = pt.add_child(upperSectionName, cc);
		child.push_back(std::make_pair(XCOUNTLS_Util::KEY_TIME, data.timeValue));
		std::string interfaceCountStr ="";
		interfaceCountStr = XCOUNTLS_Util::intToString(no_of_intrface);
		child.push_back(std::make_pair(XCOUNTLS_Util::KEY_INTERFACECOUNT, interfaceCountStr));
	}


	//todo USE IT before saveBoardCounters !!!
	/*
	if ((retVal = prepareForWriting()) != EXECUTED)
	{
		return retVal;
	}
	*/


	//todo import exit code !!
	//???
	//if ((retVal = write(upperSectionName, XCOUNTLS_Util::KEY_TIME, data.timeValue)) == EXECUTED)
	{
		for (int i=0;i<no_of_intrface;i++)
		{
			std::stringstream ss;
			if(isSMXFlag)
				ss << ( i + 1 +TRANSPORTX_START_IFACE_VALUE);
			else
				ss << ( i + 1 );


			std::string sectionName = upperSectionName+":"+ ss.str();

			//create a session for each inteface
			ptree &child = pt.add_child(sectionName, cc);

			//create the elements into session
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_INTERFACE, data.interfaceName[i]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER1, data.counterValue[i][0]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER2, data.counterValue[i][1]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER3, data.counterValue[i][2]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER4, data.counterValue[i][3]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER5, data.counterValue[i][4]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER6, data.counterValue[i][5]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER7, data.counterValue[i][6]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER8, data.counterValue[i][7]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER9, data.counterValue[i][8]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER10, data.counterValue[i][9]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER11, data.counterValue[i][10]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER12, data.counterValue[i][11]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER13, data.counterValue[i][12]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER14, data.counterValue[i][13]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER15, data.counterValue[i][14]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER16, data.counterValue[i][15]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER17, data.counterValue[i][16]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER18, data.counterValue[i][17]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER19, data.counterValue[i][18]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER20, data.counterValue[i][19]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER21, data.counterValue[i][20]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER22, data.counterValue[i][21]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER23, data.counterValue[i][22]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER24, data.counterValue[i][23]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER25, data.counterValue[i][24]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER26, data.counterValue[i][25]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER27, data.counterValue[i][26]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER28, data.counterValue[i][27]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER29, data.counterValue[i][28]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER30, data.counterValue[i][29]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER31, data.counterValue[i][30]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER32, data.counterValue[i][31]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER33, data.counterValue[i][32]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER34, data.counterValue[i][33]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER35, data.counterValue[i][34]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER36, data.counterValue[i][35]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER37, data.counterValue[i][36]));
			child.push_back(std::make_pair(XCOUNTLS_Util::KEY_COUNTER38, data.counterValue[i][37]));

	//		 ???????
	//		if(i!=33)
	//		{
	//			sectionName = upperSectionName+":"+ data.interfaceName[i+1];
	//		}

		}

	}

	return retVal;

}



int FIXS_CCH_PCDataStorage::saveBaseline(){
//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " in SaveBase "<< std::endl;

	int res = 0;

	::remove(m_tmpCFGFileName.c_str());
	try {
		boost::property_tree::write_ini(m_tmpCFGFileName.c_str(), pt);
	}

	catch (boost::property_tree::ini_parser::ini_parser_error& e) {
		std::cout << "Exception Occurred " << e.what() << std::endl;
		res = 1;
	}
	catch (boost::exception&) {
		res = 1;
	}

	pt.empty(); //to check
//	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " out of SaveBase "<< std::endl;
	return res;
}

bool FIXS_CCH_PCDataStorage::commitChanges ()
{
	int res = 0;
	if (checkFileExists()) {
		res = ::remove(m_cfgFileName.c_str());
		//std::cout << "remove cfg - " << res << " - " << m_cfgFileName.c_str() << std::endl;
	}

	res = XCOUNTLS_Util::filecopy(m_tmpCFGFileName.c_str(), m_cfgFileName.c_str());
	//std::cout << "copy tmp - " << res << std::endl;
	if (res == 0)
	{
		res = ::remove(m_tmpCFGFileName.c_str());
		//std::cout << "remove tmp - " << res << " - " << m_tmpCFGFileName.c_str() << std::endl;
	}
	return true;
}

//int FIXS_CCH_PCDataStorage::prepareForWriting ()
//{
//	int retVal =EXECUTED;
//	int res = ::remove(m_tmpCFGFileName.c_str());
//
//	if (res != 0) {
//		std::cout << "Remove failed ! - " << res << std::endl;
//	}
//
//	res = XCOUNTLS_Util::filecopy(m_cfgFileName.c_str(), m_tmpCFGFileName.c_str());
//	if (res != 0) {
//		std::cout << "copy failed ! - " << res << std::endl;
//	}
//
//	return 0;
//}

bool FIXS_CCH_PCDataStorage::deletePCDataStorageFile ()
{
	//force file deletion
	int res = ::remove(m_cfgFileName.c_str());
	res = ::remove(m_tmpCFGFileName.c_str());

	return true;
}

bool FIXS_CCH_PCDataStorage::checkFileExists()
{
	if (::access(m_cfgFileName.c_str(), F_OK) != 0) {
		//std::cout << "File not found !" << std::endl;
		return false;
	}
	return true;
}



int FIXS_CCH_PCDataStorage::saveBaselineHeading()
{
	//int retVal = EXECUTED;
	std::string currentDate;
	std::string currentTime;
	XCOUNTLS_Util::getCurrentTime(currentDate, currentTime);
	currentDate = currentDate + " " + currentTime;
	ptree cc;
	ptree &child = pt.add_child(XCOUNTLS_Util::BASELINE, cc);
	child.push_back(std::make_pair(XCOUNTLS_Util::KEY_TIME, currentDate));
	return 0;
}



bool FIXS_CCH_PCDataStorage::readBaselineHeading(std::string &currentTime)
{
	std::string sectionName = "BASELINE";
	ptree::assoc_iterator it = pt.find(sectionName);

	if (it != pt.not_found()) {
		ptree child = it->second;
		//should be ONLY ONE !!!!!!!!!!!!!!!
		for (ptree::const_iterator itc = child.begin(); itc != child.end(); ++itc) {
			std::string dummy;
			ptree childvalue = itc->second;
			currentTime = childvalue.get_value(dummy);
		}
	} else {
		currentTime = "";
		return false;
	}
	return true;
}

int FIXS_CCH_PCDataStorage::getaddrinfo(const char* /*nodename*/, std::string & /*addressStr*/)
{
	//Old method to get AP IP Address

	return 0;
	/*
    // Declare and initialize variables
	int code = EXECUTED;
	char str[50] = {0};
    WSADATA wsaData;
    int iResult;

    DWORD dwRetval;

    int i = 1;

    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;

    struct sockaddr_in  *sockaddr_ipv4;

    char *ipstringbuffer = 0;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        return INTERNALERR;
    }

    //--------------------------------
    // Setup the hints address info structure
    // which is passed to the getaddrinfo() function
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

//--------------------------------
// Call getaddrinfo(). If the call succeeds,
// the result variable will hold a linked list
// of addrinfo structures containing response
// information
	dwRetval = ::getaddrinfo(nodename, NULL, &hints, &result);
    if ( dwRetval != 0 ) {
        WSACleanup();
        return INTERNALERR;
    }

    // Retrieve each address
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
        switch (ptr->ai_family) {
            case AF_INET:
                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
				ipstringbuffer = inet_ntoa(sockaddr_ipv4->sin_addr);
				addressStr = ipstringbuffer;
				code = EXECUTED;
                break;
            default:
				code = INTERNALERR;
                break;
        }
    }

    freeaddrinfo(result);
    WSACleanup();

    return code;
    */
}

int FIXS_CCH_PCDataStorage::loadBaseline()
{

	int res = 0;

	pt.empty(); //clear structure !
	try {
		boost::property_tree::read_ini(m_cfgFileName.c_str(), pt);
	}
	catch (boost::property_tree::ini_parser::ini_parser_error& e)
	{
		//std::cout << "Ini Parser Exception Occurred " << e.what() << std::endl;
		res = 1;
	}
	catch (boost::exception& )
	{
		//std::cout << "Unknown Exception Occurred" << std::endl;
		res = 1;
	}

	return res;
}


bool FIXS_CCH_PCDataStorage::readBoardCounters (unsigned long magazine, unsigned short slot, XCOUNTLS_Util::PCData &data, int (&no_of_intrface)[2], bool isSMX)
{
	bool result = true;

	std::string upperSectionName1="", upperSectionName2="";
	std::string upperSectionName = XCOUNTLS_Util::getPcSectionName(magazine, slot);

	string uSectionName[2]={"\0","\0"};
	std::string interfaceCountStr[2];
	if(isSMX){
		upperSectionName1= upperSectionName +"_C";
		uSectionName[0] = upperSectionName1; //Now the 0th location should be BS:15_2_0_4:0_C
		upperSectionName2= upperSectionName +"_T";
		uSectionName[1] = upperSectionName2; //Now the 1th location should be BS:15_2_0_4:0_T
	}else{
		uSectionName[0] = upperSectionName+"_C";
	}

	for(int cntr=0; cntr<(sizeof(uSectionName)/sizeof(*uSectionName)) ;cntr++)
	{ //should iterate for 2 times
		if(strcmp(uSectionName[cntr].c_str(),"\0")==0){
			break;
		}

	ptree::assoc_iterator it_time = pt.find(uSectionName[cntr]);

	if (it_time != pt.not_found()) {
		ptree child = it_time->second;
		//should be ONLY ONE !!!!!!!!!!!!!!!
		int j=0;
		for (ptree::const_iterator itc = child.begin(); itc != child.end(); ++itc) {
			std::string dummy;
			ptree childvalue = itc->second;
			if(j==0)
			{
			    data.timeValue = childvalue.get_value(dummy);
			}
			else if(j==1)
			{
				interfaceCountStr[cntr] = childvalue.get_value(dummy);
			}
			j++;
				
		}

	} else {

		data.timeValue = "";
		interfaceCountStr[cntr] = "";
	       }
		if(interfaceCountStr[cntr] == "")
		{
				if((slot == 26)||(slot == 28))
					no_of_intrface[cntr] = 36;  //CMX interface count is 36
				else if(strcmp(upperSectionName2.c_str(),uSectionName[cntr].c_str())==0)
					no_of_intrface[cntr] = SMX_INTERFACE_COUNT;
				else
					no_of_intrface[cntr] = 35;  //SCX interface count is 35

		}
		else
		{
			int intrfc_cnt=-1;
			XCOUNTLS_Util::stringToInt (interfaceCountStr[cntr], intrfc_cnt); // should consist of interfaceCountStr[0]=34 & interfaceCountStr[1]=43
			no_of_intrface[cntr]=intrfc_cnt;
		}


	{
		int i=0;
		int k=0;
		for(i=0;i< no_of_intrface[cntr];i++)
		{
			std::stringstream ss;
			std::string dummy;
			if((strcmp(upperSectionName2.c_str(),uSectionName[cntr].c_str())==0) && (isSMX)) {
				k=(i + 1 + TRANSPORTX_START_IFACE_VALUE);
				ss << k;
			}else{
				k=i+1;
				ss << k;
			}

			std::string sectionName = uSectionName[cntr]+":"+ ss.str();

			ptree::assoc_iterator it = pt.find(sectionName);

			if (it != pt.not_found()) {
				//This element will contain all section element [BS:76_4_0_0:0_C:1]
				ptree child = it->second;

				//running through the children
				int j = -1;
				for (ptree::const_iterator itc = child.begin(); itc != child.end(); ++itc) {
					if (j == -1) {
						ptree childvalue = itc->second;
						data.interfaceName[k-1] = childvalue.get_value(dummy);
					} else if (j < MAX_NO_OF_COUNTERS) {
						ptree childvalue = itc->second;
						data.counterValue[k-1][j] = childvalue.get_value(dummy);
					} else {
						//too much child !?!?
					}
					j++;
				}
			} else {
				//empty all
				result = false;
				//std::cout << " NOT FOUND !!!! " <<  std::endl;
				for (int j = 0 ; j < MAX_NO_OF_COUNTERS ; j++) {
					data.interfaceName[k-1] = "";  //data starts from 0th location
					data.counterValue[k-1][j] = "";
				}
			}

			//????????
			/*
			if(i!=33)
			{
				sectionName = upperSectionName+":"+ data.interfaceName[i+1];
			}
			*/
		}
	}
}
	return result;
}

