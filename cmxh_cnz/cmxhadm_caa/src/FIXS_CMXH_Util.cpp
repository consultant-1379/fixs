
#include "FIXS_CMXH_Util.h"
//#include "ACS_TRA_Trace.h"
#include<errno.h>


std::string CMXH_Util::ulongToStringIP (uint32_t  ipaddress)
{
  	char tempValueStr[16] = {0};
	uint32_t tempValue = ipaddress;
	snprintf(tempValueStr, sizeof(tempValueStr), "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >> 16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
    std::string IP = tempValueStr;
	return IP;
}

bool CMXH_Util::checkFigures (std::istringstream &istr) 
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

bool CMXH_Util::stringToInt (std::string value, int &ivalue)
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

int CMXH_Util::createDir(std::string dirName) 
{
        /*HW55782 - changed chdir to stat */
	struct stat st;
        if(stat(dirName.c_str(),&st)== -1)
	{ //ERROR

		if (int resMk = ::mkdir(dirName.c_str(),  0777 ) ) {
			std::cout << "\n DBG: Creation failed res: " << resMk << std::endl;
			return -1;
		} else {
			std::cout << "\n DBG: Dir successfully created " << resMk << std::endl;

			if (int resChmod = ::chmod(dirName.c_str(), 0777 ) ) {
				std::cout << "\n DBG: resChmod " << resChmod << std::endl;
				return -1;
			} else {
				std::cout << "\n DBG: Chmod ok " << resChmod << std::endl;
			}
		}
	} 
	else 
	{
		std::cout << "\n DBG: Already presents: " << std::endl;

		if (int resChmod = ::chmod(dirName.c_str(), 0777 ) ) {
			std::cout << "\n DBG: resChmod " << resChmod << std::endl;
			return -1;
		} else {
			std::cout << "\n DBG: Chmod ok" << resChmod << std::endl;
		}
	}
	return 0;
}

int CMXH_Util::getNode()
{
        std::string node_id_path = "/etc/cluster/nodes/this/id";

        fstream nodeFile;
        nodeFile.open(node_id_path.c_str(), ios::in);
        if (!nodeFile) {
                std::cout << "DGB: File Not found" << std::endl;
                return -1;
        }
        char val[256];
        char *c = val;
        nodeFile.getline(c, 256);
        std::cout << "DBG: File " << node_id_path.c_str() << " Row: " << c << std::endl;
        nodeFile.close();
        return ::atoi(c);
}

void CMXH_Util::renameCopiedContFile (std::string  contFileName, std::string ipNumber)
{
	std::string tftpPathAPG = "/data/apz/data/boot/cmx/cmx_logs";
	//std::string cmxIP = ipNumber.substr(12,(ipNumber.size()-1));
	
	std::string clusterIP = "192.168.169.";
/*	std::string tmp(ipNumber);

	int nodeNum = CMXH_Util::getNode();

	if (clusterIP.compare(tmp.substr(0,12)) == 0 ) {
		char c[16];
		sprintf(c, "%d", nodeNum);
		clusterIP += std::string(c);
	} else {
		clusterIP = "192.168.170.";
		char c[16];
		sprintf(c, "%d", nodeNum);
		clusterIP += std::string(c);
	} */

	// Replace (.) with (_) in Ip number	
	std::string ipFiltered(ipNumber);
	if (ipFiltered.compare("")!=0)
	{
		size_t foundDot;
		foundDot = ipFiltered.find_first_of(".");
		while (foundDot != std::string::npos){
			ipFiltered[foundDot] = '_';
			foundDot = ipFiltered.find_first_of(".",foundDot+1 );
		}
	}

	std::string tftpPathAPGOld = tftpPathAPG + "/cmx_" + ipFiltered + "/";
	tftpPathAPGOld.append(contFileName);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "File name before rename: "<<tftpPathAPGOld.c_str()<<std::endl;

	char byte[512];
	memset(byte, 0xFF, sizeof(byte));

	time_t longTime;
	time(&longTime);  // Get the current time
	

	struct tm* today = localtime(&longTime); 
	int year = 1900 + today->tm_year;
	sprintf(byte, "%u%02u%02u_%02u%02u%02u",year, (today->tm_mon + 1) ,today->tm_mday,today->tm_hour, today->tm_min,today->tm_sec );
	std::cout<<" FIXS_CMXH_SNMPManager::renameCopiedContFile byte "<< byte<<std::endl;

	std::string tftpPathAPGNew = tftpPathAPG + "/cmx_" + ipFiltered + "/";
	tftpPathAPGNew.append(contFileName);
	size_t pos;
	pos = tftpPathAPGNew.find(".tar");
	tftpPathAPGNew = tftpPathAPGNew.substr (0,pos);
	tftpPathAPGNew.append("_");
	tftpPathAPGNew.append(byte);
	tftpPathAPGNew.append(".tar.gnupg.gz");
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "File name after rename: "<<tftpPathAPGNew.c_str()<<std::endl;
	
	int retValue = rename(tftpPathAPGOld.c_str(), tftpPathAPGNew.c_str());

	if( retValue == 0)
		std::cout<<"FIXS_CMXH_SNMPManager::renameCopiedContFile File successfully renamed"<<std::endl;
	else
	{
		std::cout<<"FIXS_CMXH_SNMPManager::renameCopiedContFile File rename failed with : "<<strerror(errno) << std::endl;
	}
}

uint64_t CMXH_Util::getEpochTimeInSeconds()
{
	uint64_t timeinseconds;
	std::string secInStr;

	time_t sec;
	sec = time(NULL);  // Get the current time in epch sec

	timeinseconds = static_cast<uint64_t> (sec);

	return timeinseconds;
}

int CMXH_Util::getDRBDVlan()
{
	std::string storage_api = "/usr/share/pso/storage-paths/config";
	fstream ifs;
	ifs.open(storage_api.c_str(), ios::in);
	if (!ifs) {
		std::cout << "DGB: [" << storage_api.c_str() << "] File Not found" << std::endl;
		return -1;
	}
	char val[256];
	char *c = val;
	ifs.getline(c, 256);
	std::cout << "DBG: File " << storage_api.c_str() << " Row: " << c << std::endl;
	ifs.close();

	std::string fileName = c;
	fileName.append("/apos/network_10g_vlantag");
   	ifs.open(fileName.c_str(), ios::in);
  	if (!ifs) {
   		std::cout << "DGB: [" << fileName.c_str() << "] File Not found" << std::endl;
       	return -1;
	}
	memset(val, 0, 256);
   	ifs.getline(c, 256);
   	std::cout << "DBG: File " << fileName.c_str() << " Row: " << c << std::endl;
   	ifs.close();
   	return ::atoi(c);
}

