/*
 * cmd_Utils.cpp
 *
 *  Created on: Feb 29, 2012
 *      Author: eanform
 */

#include "cmd_Utils.h"
#include "ACS_CS_API.h"
bool CMD_Util::getImmAttributeString (std::string object, std::string attribute, std::string &value)
{
	bool res = true;
	ACS_CC_ReturnType result;
	OmHandler omHandler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{
		ACS_CC_ImmParameter Param;
		//Param.ACS_APGCC_IMMFreeMemory(1);
		char *name_attrPath = const_cast<char*>(attribute.c_str());
		Param.attrName = name_attrPath;
		result = omHandler.getAttribute(object.c_str(), &Param );
		if ( result != ACS_CC_SUCCESS ){	/*cout << "ERROR: Param " << attribute.c_str()<<" FAILURE!!!\n";*/ res = false; }
		else value = (char*)Param.attrValues[0];

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
//			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
			res = false;
		}
	}

	return res;
}


bool CMD_Util::getImmAttributeInt(std::string object, std::string attribute, int &value)
{
 	bool res = true;
 	ACS_CC_ReturnType result;
 	OmHandler omHandler;

 	result = omHandler.Init();
 	if (result != ACS_CC_SUCCESS)
 	{
// 		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
 		res = false;
 	}
 	else
 	{
 		ACS_CC_ImmParameter Param;
 		//Param.ACS_APGCC_IMMFreeMemory(1);
 		char *name_attrPath = const_cast<char*>(attribute.c_str());
 		Param.attrName = name_attrPath;
 		result = omHandler.getAttribute(object.c_str(), &Param );
 		if ( result != ACS_CC_SUCCESS ){	/*cout << "ERROR: Param " << attribute.c_str()<<" FAILURE!!!\n";*/ res = false; }
 		else value = (*(int*)Param.attrValues[0]);

 		result = omHandler.Finalize();
 		if (result != ACS_CC_SUCCESS)
 		{
// 			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
 			res = false;
 		}
 	}

 	return res;
}

bool CMD_Util::CopyFileToDataDisk(std::string & pathFile, std::string & destFile)
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
//		std::cout << "DBG: CopyFile("<<pathFile.c_str()<<","<<destFile.c_str()<<")"<<std::endl;


		if (CMD_Util::CopyFile(pathFile.c_str(),destFile.c_str(),false,size) == false )
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

std::string CMD_Util::replaceSubstr( const std::string& content, const std::string& instance, const std::string& replacement )
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

CMD_Util::EnvironmentType CMD_Util::getEnvironment(void)
{
	CMD_Util::EnvironmentType env;
	ACS_CS_API_NS::CS_API_Result resp = ACS_CS_API_NS::Result_Failure;
	ACS_CS_API_CommonBasedArchitecture::ArchitectureValue arch;
	bool multipleCPSystem = false;

	if ((ACS_CS_API_NetworkElement::isMultipleCPSystem(multipleCPSystem) == ACS_CS_API_NS::Result_Success) && (ACS_CS_API_NetworkElement::getNodeArchitecture(arch) == ACS_CS_API_NS::Result_Success))
	{
		switch(arch)
		{
		case ACS_CS_API_CommonBasedArchitecture::SCB:

			if (multipleCPSystem) env = MULTIPLECP_NOTCBA;
			else env = SINGLECP_NOTCBA;
			break;

		case ACS_CS_API_CommonBasedArchitecture::SCX:

			if (multipleCPSystem) env = MULTIPLECP_CBA;
			else env = SINGLECP_CBA;
			break;

		case ACS_CS_API_CommonBasedArchitecture::DMX:

			if (multipleCPSystem) env = MULTIPLECP_DMX;
			else env = SINGLECP_DMX;
			break;
		case ACS_CS_API_CommonBasedArchitecture::VIRTUALIZED:
			
			env = VIRTUALIZED;
			break;
		case ACS_CS_API_CommonBasedArchitecture::SMX:

			if (multipleCPSystem) env = MULTIPLECP_SMX;
			else env = SINGLECP_SMX;
			break;
		default:
			env = UNKNOWN;
			break;
		}
	}
	else
	{
		env = UNKNOWN;
	}

	return env;

}


std::string CMD_Util::GetAPHostName()
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


bool CMD_Util::CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists, long int size)
{
   if ( bFailedIfExists && CMD_Util::PathFileExists(szDestFile) )
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



bool CMD_Util::CopyFile(const char* szSourceFile, const char* szDestFile, bool bFailedIfExists)
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
	   cout << "bFailedIfExists and  PathFileExists(szDestFile"<<endl;
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
	   cout << "could not fopen sourcefile "<<endl;
      return false;
   }

   pDestFile = fopen(szDestFile, "wb");

   if ( !pDestFile )
   {
	   cout <<"Could not fopen dest file"<<endl;
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
        	 cout <<"byte count not matching"<<endl;

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


bool CMD_Util::PathFileExists(const char* file)
{
	struct stat st;
		//check if exist
		if(stat(file,&st) != 0)	return false;
		else return true;

}


int CMD_Util::findFile(char* path, char* type, std::vector<std::string> &result)
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


//	if(new_path!=NULL)
//	{
//		free(new_path);
//		new_path=NULL;
//	}

	//close directory

	if(closedir(dp) < 0){

		printf("\nclosedir error on path %s\n", path);

	}

	free(dirp);


	return resu;

}




int CMD_Util::removeFolder(const char* _fullpath){

	DIR		*dp = NULL;
	struct dirent	*dirp = NULL;
	struct stat 	statbuf;
	char		*new_fullpath = NULL;
	int 		ret;
	std::string strTmp;

	if (stat(_fullpath,&statbuf) != 0)
	{
//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Direcoty: "<< _fullpath << " doesn't exist !!! "<< std::endl;
		return 0;
	}

	if(!S_ISDIR(statbuf.st_mode))
	{
//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " Is not a Direcoty: "<< _fullpath << std::endl;

		std::string pathToString = std::string (_fullpath);

		int iPos = pathToString.find_last_of(DIR_SEPARATOR);
		if(iPos >=0)
		{
			// remove filename
			strTmp = pathToString.substr(0,iPos);
			_fullpath = strTmp.c_str();
		}
		else return LSTAT_ERROR;

	}


	if((dp = opendir(_fullpath)) == NULL)
	{
//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Read permission is denied for the directory named: "<< _fullpath << std::endl;
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
//					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "closedir "<< std::endl;
					return CLOSEDIRR_ERROR;
				}
				return ret;
			}
		}
		//remove file
		else {
			if(remove(new_fullpath) < 0){
				free(new_fullpath);
				closedir(dp);
				return REMOVE_ERROR;
			}
//				else std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Remove file "<< std::endl;
		}

		free(new_fullpath);

	}// end-while

	//close directory
	if(closedir(dp) < 0){
//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "closedir "<< std::endl;
		return CLOSEDIRR_ERROR;
	}

	//remove directory
	if(remove(_fullpath) < 0){
//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "remove "<< std::endl;
		return REMOVE_ERROR;
	}

	return 0;

}// end-removeFolder

bool CMD_Util::CheckExtention(const char* file, const char* type_file)
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

void CMD_Util::findAndReplace( string &source, const string find, string replace )
{
    size_t j;
    for ( ; (j = source.find( find )) != string::npos ; )
    {
        source.replace( j, find.length(), replace );
    }
}


void CMD_Util::trim(string& str)
{
	string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
}

void CMD_Util::findAndRemove( string &source, const string find)
{
	size_t j;
	for ( ; (j = source.find( find )) != string::npos ; )
	{
		source.erase(j,1);
	}
}

bool CMD_Util::checkFigures (std::istringstream &istr)
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

bool CMD_Util::stringToInt (std::string value, int &ivalue)
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

std::string CMD_Util::intToString (int ivalue)
{
	std::stringstream ss_value("");
	ss_value << ivalue; //try to convert operand to value number
	return ss_value.str();
}

std::string CMD_Util::getNbiSwPath(fileMFolderType folderType)
{

	std::string completeFolderPath("");
	char p_finaloutputdir[100];
	int len=100;
	ACS_APGCC_CommonLib apgccCommon;
	ACS_APGCC_DNFPath_ReturnTypeT retcode;
	const char * p_FileMFuncName = ATT_FILEM_SWPACKAGE_PATH;

	switch (folderType) {

		case swPackage:

			p_FileMFuncName = ATT_FILEM_SWPACKAGE_PATH;
			break;

		case swPackageScxb:

			p_FileMFuncName = ATT_FILEM_SWPACKAGE_PATH_SCXB;
			break;

		case swPackageEpb1:

			p_FileMFuncName = ATT_FILEM_SWPACKAGE_PATH_EPB1;
			break;

		case swPackageIptb:

			p_FileMFuncName = ATT_FILEM_SWPACKAGE_PATH_IPTB;
			break;

		case swPackageCmxb:

			p_FileMFuncName = ATT_FILEM_SWPACKAGE_PATH_CMXB;
			break;

		case swPackageEvoEt:

			p_FileMFuncName = ATT_FILEM_SWPACKAGE_PATH_EVOET;
			break;

		case swPackageFw:

			p_FileMFuncName = ATT_FILEM_SWPACKAGE_PATH_FW;
			break;

		default:
			break;

	}

	retcode = apgccCommon.GetFileMPath(p_FileMFuncName,p_finaloutputdir,len);
	if( retcode != ACS_APGCC_DNFPATH_SUCCESS)
	{
//		std::cout<<"Error in getFileMPath : " << retcode << std::endl;
	}


	completeFolderPath = p_finaloutputdir;


	return completeFolderPath;
}


bool CMD_Util::getChildrenObject(std::string dn, std::vector<std::string> & list )
{
	ACS_CC_ReturnType result;
	bool res = true;
	OmHandler omHandler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{

		result = omHandler.getChildren(dn.c_str(),ACS_APGCC_SUBLEVEL, &list);
		if (result != ACS_CC_SUCCESS)
		{
//			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error getChildren" << std::endl;
			res = false;
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
//			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}

	return res;
}


bool CMD_Util::getClassObjectsList(std::string className, std::vector<std::string> & list )
{
	ACS_CC_ReturnType result;
	bool res = true;
	OmHandler omHandler;

	result = omHandler.Init();
	if (result != ACS_CC_SUCCESS)
	{
//		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Init()" << std::endl;
		res = false;
	}
	else
	{

		result = omHandler.getClassInstances(className.c_str(),list);
		if (result != ACS_CC_SUCCESS)
		{
//			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error getClassIstance" << std::endl;
			res = false;
		}

		result = omHandler.Finalize();
		if (result != ACS_CC_SUCCESS)
		{
//			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error Finalize()" << std::endl;
			res = false;
		}
	}

	return res;
}


bool CMD_Util::getRdnAttribute (std::string object_dn, std::string &rdn)
{
	bool result = false;
	if (!object_dn.empty())
	{
		rdn = ACS_APGCC::before(object_dn,"=");
		result = true;
	}
	return result;
}

bool CMD_Util::stringToUlongMagazine (std::string value, unsigned long &uvalue)
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

bool CMD_Util::stringToUshort (std::string value, unsigned short &uvalue)
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

	if (uvalue > 28 || uvalue == 27 )
	{
		return false; //Usage
	}

	return true;
}

unsigned short CMD_Util::numOfChar (std::istringstream &istr, char searchChar)
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

void CMD_Util::reverseDottedDecStr (std::istringstream &istr)
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

bool CMD_Util::dottedDecToLong (std::istringstream &istr, unsigned long &value, unsigned long  lowerLimit1, unsigned long  upperLimit1, unsigned long  lowerLimit2, unsigned long  upperLimit2, unsigned long  lowerLimit3, unsigned long  upperLimit3, unsigned long  lowerLimit4, unsigned long  upperLimit4)
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


bool CMD_Util::stringToUpper (std::string& stringToConvert)
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

std::string CMD_Util::ulongToStringIP (unsigned long ipaddress)
{
	char tempValueStr[16] = {0};
	uint32_t tempValue = ipaddress;
	snprintf(tempValueStr, sizeof(tempValueStr), "%u.%u.%u.%u", (tempValue >> 24) & 0xFF, (tempValue >> 16) & 0xFF, (tempValue >> 8) & 0xFF, tempValue & 0xFF);
    std::string IP = tempValueStr;
	return IP;
}
int CMD_Util::getApubBoard (std::string magazine, unsigned short& slot)
{
        unsigned long umagazine;
        unsigned short boardId;
        stringToUlongMagazine(magazine,umagazine);
        int retValue = -1;
        ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
        if (hwc)
        {
                ACS_CS_API_BoardSearch * boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
                if (boardSearch)
                {
                        ACS_CS_API_IdList boardList;
                        boardSearch->setMagazine(umagazine);
			 boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_APUB);
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
					BoardID boardId = boardList[0];
                                        returnValue = hwc->getSlot (slot, boardId);
                                        if (returnValue == ACS_CS_API_NS::Result_Success)
                                        {
                              
                                        retValue = 1;
					}
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
