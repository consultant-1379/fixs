
#ifndef FIXS_CCH_DiskHandler_h
#define FIXS_CCH_DiskHandler_h 1

#include "FIXS_CCH_Util.h"
#include "FIXS_CCH_SoftwareMap.h"
#include <string>
#include "ACS_APGCC_Util.H"
#include "ACS_APGCC_CommonLib.h"
#include "ace/OS_NS_dirent.h"
#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include <unistd.h>

class FIXS_CCH_DiskHandler
{


  public:

      static bool deleteTempFiles ();

      static void deleteFile (const char *file);

      static void createDefaultFolder (CCH_Util::EnvironmentType environment);

      static void removeSoftwarePackage (const char *softwarePackage);

      static std::string getScxFolder ();

      static std::string getIptFolder ();

      static std::string getIplbFolder ();     
	  
      static std::string getSmxFolder ();

      static std::string getTftpRootFolder ();

      static std::string getTftpRootFolderDefault ();

      static bool copyIptSoftware(const char* srcFolder,const char* dstFolder);

      static std::string getEpb1Folder ();

      static std::string getConfigurationFolder ();

      static std::string getTempConfigurationFile ();

      static std::string getConfigurationFile ();

      static std::string getNbiSwPath(CCH_Util::fileMFolderType folderType);

      static CCH_Util::EnvironmentType getEnvironment();

      static void setEnvironment(CCH_Util::EnvironmentType env);

      static ACS_TRA_Logging * getLogInstance ();

      static void closeLogInstance ();

      static void checkStartAfterRestore();

      static bool isStartedAfterRestore();

      static std::string getCmxFolder ();

      static std::string getEvoEtFolder ();

      static void createMisallingnedSwObjects(std::vector<std::string> & p_dnList,std::vector<std::string> & immCxpObject);
protected:
private:
    FIXS_CCH_DiskHandler();
    FIXS_CCH_DiskHandler(const FIXS_CCH_DiskHandler & right);
    virtual ~FIXS_CCH_DiskHandler();
    FIXS_CCH_DiskHandler & operator =(const FIXS_CCH_DiskHandler & right);
    static void createIplbDefaultDirectory(ACS_TRA_trace* _trace);
    static void createScxDefaultDirectory(ACS_TRA_trace* _trace);
    static void createEpblDefaultDirectory(ACS_TRA_trace* _trace);
    static void createIptDefaultDirectory(ACS_TRA_trace* _trace);
    static void createCmxDefaultDirectory(ACS_TRA_trace* _trace);
    static void createEvoEtDefaultDirectory(ACS_TRA_trace* _trace);
    static void createIpmiupgDefaultDirectory(ACS_TRA_trace* _trace);
    static void createSmxDefaultDirectory(ACS_TRA_trace* _trace);
    static CCH_Util::EnvironmentType m_environment;
    static std::string m_scxFolder;
    static std::string m_iptFolder;
    static std::string m_tftpRootFolder;
    static std::string m_tftpRootFolderDefault;
    static std::string m_epb1Folder;
    static std::string m_cmxFolder;
    static std::string m_evoEtFolder;
    static std::string m_iplbFolder;
    static std::string m_apzFolder;
    static std::string m_ipmiupgFolder; 
    static ACS_TRA_Logging *FIXS_CCH_logging;
    static bool startAfterRestore;
    static std::string m_smxFolder;
private:
};

// Class FIXS_CCH_DiskHandler

#endif
