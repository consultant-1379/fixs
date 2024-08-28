#ifndef FIXS_CCH_Installer_h
#define FIXS_CCH_Installer_h 1

#include "Client_Server_Interface.h"
#include "FIXS_CCH_DiskHandler.h"
#include "FIXS_CCH_SoftwareMap.h"
#include "FIXS_CCH_Util.h"

#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"
#include "ACS_APGCC_Util.H"
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>

#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/SAXParser.hpp>

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/HandlerBase.hpp>

namespace
{
/*enum respCode
{
	//upgradepackage
	EXECUTED				= 0,	//EXECUTED
	INTERNALERR				= 1,	// Generic internal error
	INCORRECTUSAGE			= 2,	// Usage
	DIRECTORYERR			= 3,	// "Directory Error" Internal Error during filesystem managing
	PACKAGE_IN_USE			= 4,	// "SW Package in use" The package is used then it can not be removed
	PACKAGE_NOT_EXIST		= 5,	// "SW Package not exists" The package does not exist
	XMLERROR     			= 6,	// "XML Error" XML is not formatted properly
	NO_REPLY_FROM_CS		= 7,    // "No reply from CS" cannot fetch information from CS

	UPGRADE_ALREADY_ONGOING		= 8,	// "Upgrade already ongoing" : an upgrade is already ongoing on specified board or magazine
	SW_ALREADY_ON_BOARD			= 9,	// "Specified SW already present on board"
	WRONG_OPTION				= 10,	// "Wrong option in this state"
	PACKAGE_ALREADY_INSTALLED	= 11,	// "SW package already installed"
	UPGRADE_ACTIVATE_FAILED		= 12,	// "Activate failure"
	SNMP_FAILURE				= 13,	// "SNMP Failure"
	HARDWARE_NOT_PRESENT		= 14,	// "Hardware not present"
	PACKAGE_NOT_INSTALLED		= 15,	// "Package not installed"
	INCORRECT_PACKAGE			= 16,	// "Package not correct"
	WAIT_FOR_UPGRADE_ONGOING	= 17,	// "Display command is inhibited during upgrade fw ongoing"
	NOT_ALLOWED_UPGRADE_ONGOING	= 18,	// "Delete option is inhibited during upgrade sw ongoing"
	FUNC_BOARD_NAME_ERROR		= 19,	// "Functional Board name error"
	PACKAGE_IS_DEFAULT			= 20,	// "SW Package in default" The package is setted as default then it can not be removed
	MAXIMUM_LOAD_MODULES_INSTALLED	= 21,	// "Maximum Number of Load Modules are already Installed"
	FUNC_BOARD_NOTALLOWED		= 22,	// "Not Allowed for this Functional Board Name"
	SLOT_FBN_NOT_SAME			= 23, //"All slots specified do not contain same type of board"
	DISK_FULL					= 24, //"Disk quota exceeded"
	PACKAGE_NOT_APPLICABLE		= 25, //"Load Module not Applicable for this Functional Board Name".

	//Boardreset codes
	INCORRECT_MAGAZINE			= 30, //"Incorrect magazine number"
	INCORRECT_SLOT				= 31, //"Incorrect slot number"
	INCORRECT_RESPONSE			= 32, //"Illegal value - enter Yes/No (or) Y/N" Incorrect response to confirmation question
	BOARD_BLOCKED_NO_RESET      = 33,   //Board cannot be reset as it is blocked
	BOARD_ALREADY_DEBLOCKED		= 34,	//Board in not in blocked state.So need not deblock
	BOARD_ALREADY_BLOCKED		= 35,	//Board is already blocked

	ONLY_ALLOWED_FROM_IO_APG	= 40,	//Only allowed from IO APG

	// Performance Counter codes
	NO_INTERFACE_UP				= 41,
	NOT_EXIST_INTERFACE			= 42,
	BASELINE_FILE_NOT_FOUND		= 43,
	FILE_DELETION_FAILED		= 44,

	//IPMI upgrade
	IPMIUPG_READY = 50,
	IPMIUPG_ONGOING = 51,
	IPMIUPG_MIBSTATUS_READY = 52,
	IPMIUPG_MIBSTATUS_ONGOING = 53,
	IPMIUPG_MIBSTATUS_NOTSTARTED = 54,
	IPMIUPG_MIBSTATUS_UNKNOWN = 55,
	IPMIUPG_MIBTRAP_OK = 56,
	IPMIUPG_MIBTRAP_FILE_ERR = 57,
	IPMIUPG_MIBTRAP_OTHER_ERR = 58,

	//on load module upgrade
	WRONG_OPTION_PREPARE		= 59,
	WRONG_OPTION_ACTIVATE		= 60,
	WRONG_OPTION_COMMIT			= 61,
	WRONG_OPTION_CANCEL			= 62,

	//generic codes
	ILLEGAL_COMMAND	= 115,	// Illegal option in this system configuration
	UNABLECONNECT	= 117	// Unable to connect to server

};*/
}

XERCES_CPP_NAMESPACE_USE
XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END

#define MAX_PATH_CCH 1000

//
// FIXS_CCH_DiskHandler
#include "FIXS_CCH_DiskHandler.h"

class FIXS_CCH_Installer
{

  public:

    typedef enum {
        // SCX Board
        PRODUCT_NAME,
        PRODUCT_ID,
        PRODUCT_RSTATE,
        FILE_RELPATH,
        FILE_NAME,
        FILE_PROVIDER_PRODNR,
        FILE_PROVIDER_RSTATE,
        // Load Module
        VERSION_REL,
        PACKAGE_NAME,
        PACKAGE_ID,
        PACKAGE_RSTATE,
        PACKAGE_FBN,
        LM_REL_PATH,
        LM_BOOTFILE_NAME,
        LM_PRODUCT_NUMBER,
        LM_RSTATE,
	LM_SW_VER_TYPE
    } XmlProperties;

  typedef std::map<XmlProperties, std::string> xmlPropertiesMap_t;

  typedef enum {
        UNKNOWN,
        SCX_INSTALLATION,
        IPT_INSTALLATION,
        EPB1_INSTALLATION,
        EVOET_INSTALLATION,
        CMX_INSTALLATION,
        IPLB_INSTALLATION,
        SMX_INSTALLATION
    } InstallationType;

    class SaxHandler : public HandlerBase
    {

		typedef std::map<XmlProperties, std::string> propertiesMap_t;

      public:

          SaxHandler (const std::string &xmlFilePath, propertiesMap_t &xmlProperties, InstallationType& installationType);


          virtual ~SaxHandler();

          //	Method to handle the start events that are generated for each element when
          //	an XML file is parsed
          void startElement (const XMLCh* const  name, AttributeList &attributes);


          //	Metod to handle errors that occured during parsing
          void warning (const SAXParseException &exception);


          //	Metod to handle errors that occured during parsing
          void error (const SAXParseException &exception);


          //	Metod to handle errors that occured during parsing
          void fatalError (const SAXParseException &exception);

        // Additional Public Declarations

      protected:
        // Additional Protected Declarations

      private:

          SaxHandler(const SaxHandler &right);

          SaxHandler & operator=(const SaxHandler &right);

          // Data Members for Class Attributes

          ACS_TRA_trace *_traceSax;

          propertiesMap_t &m_xmlProperties;

          std::string m_xmlFilePath;

          FIXS_CCH_Installer::InstallationType &m_installationType;

        // Additional Private Declarations

      private:
        // Additional Implementation Declarations
    };

      FIXS_CCH_Installer (std::string archiveFullPath);
      FIXS_CCH_Installer ();
      

      virtual ~FIXS_CCH_Installer();

      int install (int fbn);

      void clean ();

      bool initialize ();

      std::string getInstalledPackage ();

      std::string getInstalledProduct ();

      std::string getFileNameInPackage ();

      int fetchPackageInfo();
      bool SetXmlFileAfterRestore(std::string xmlPath); 
      int getCmxVersionType ();

      //	remove the software package directory and clean the temporay files
      void uninstall ();

      bool findTarFile ();

      bool findZipFile ();

      FIXS_CCH_Installer::InstallationType getInstallationType ();

  public:
    // Additional Public Declarations

  protected:
    // Additional Protected Declarations

  private:

      FIXS_CCH_Installer & operator=(const FIXS_CCH_Installer &right);

      bool initializeParser ();

      bool setXmlFile ();

      bool setTarFile ();

      int installScx ();

      int installipt ();

      int installEpb1 ();

      int installCmx ();

      int installEvoEt ();
       
      int installIplb ();   
	  
      int installSmx ();

    // Data Members for Class Attributes

      ACS_TRA_trace *_trace;

      std::string m_archiveFullPath;

      std::string m_archiveSource;

      std::string m_archivePath;

      xmlPropertiesMap_t m_xmlProperties;

      char  m_xmlFile[MAX_PATH_CCH];

      std::string m_cmdFolder;

      char  m_cmdFolderTar[MAX_PATH_CCH];

      std::string m_file;

      std::string m_fileTar;

      std::string m_extract;

      std::string m_extractTar;

      //	concatenation of productId + "_" + rState

      std::string m_installedPackage;

      std::string m_installedProduct;

      std::string m_finalSourceFolder;

      std::string m_fileInPackage;

      std::string m_extractZip;

      char  m_cmdFolderZip[MAX_PATH];
	
      InstallationType m_installationType;

      int m_swVerType;

      ACS_TRA_Logging *FIXS_CCH_logging;

    // Additional Private Declarations

  private:
    // Additional Implementation Declarations
};
// Class FIXS_CCH_Installer::SaxHandler

// Class FIXS_CCH_Installer

#endif
