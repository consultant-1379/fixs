#ifndef FIXS_CCH_SoftwareMap_h
#define FIXS_CCH_SoftwareMap_h 1

//#include "ACS_APGCC_Common_Define.h"
#include "ace/OS.h"
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <bits/stl_map.h>
#include <list>
#include <string>
#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>

#include "ace/Reactor.h"
#include "ace/TP_Reactor.h"

#include "ACS_TRA_trace.h"
#include "ACS_TRA_Logging.h"

#include "FIXS_CCH_ImmInterface.h"
#include "FIXS_CCH_DiskHandler.h"
#include "Client_Server_Interface.h"

class FIXS_CCH_ImmInterface;

class FIXS_CCH_SoftwareMap
{

  public:

    class LoadModuleMapData
    {
      public:
        // Data Members for Class Attributes
          std::string m_container;

          std::string dname;

          bool m_isDefault;

          std::string m_product;

          std::string m_fbn;

          std::string m_comment;

          bool m_isUsed;

          bool m_isFaulty;

        // Additional Public Declarations

      protected:
        // Additional Protected Declarations

      private:
        // Additional Private Declarations

    };


	typedef std::map<std::string, FIXS_CCH_SoftwareMap::LoadModuleMapData> softwareMap_t;
	std::vector<std::string> immFaultySwObjects;
      static  FIXS_CCH_SoftwareMap* getInstance ();

      int initialize ();

      void finalize();

      bool isScxEmpty ();
	  
      bool isSmxEmpty ();

      bool addScxEntry (std::string container, std::string product = "-", const char* dn = "");

      void addScxEntry (LoadModuleMapData data);

      void removeScxEntry (std::string container);

      bool addSmxEntry (std::string container, std::string product = "-", const char* dn = "");

      void addSmxEntry (LoadModuleMapData data);

      void removeSmxEntry (std::string container);

      bool isContainerInstalled (std::string container);

      int size ();

      bool isDefault (std::string package);

      void getNotUsedPackages (std::list<std::string> &packages, char *fbn = "");

      void setScxDefaultPackage (std::string package);
	  
		void setSmxDefaultPackage (std::string package);

      std::string getScxDefaultPackage ();
	  
	  std::string getSmxDefaultPackage ();

      std::string getSoftwareProduct (std::string &container);

      //	Check if the CXC is installed
      bool isProductInstalled (std::string cxc, int fbn);

      //	Check if the CXC is installed into all maps
      bool isProductInstalled (std::string cxc);

      std::string getContainerByProduct (std::string &product);

      bool isUsedPackage (std::string package);

      bool isFaultyPackage (std::string package);

      bool checkFaultyContainers (std::string fbn);

      void setNotFaultyContainer (std::string container);

      bool isIptEmpty ();

      bool addIptEntry (std::string container, std::string product = "-", const char* dn = "");

      void addIptEntry (LoadModuleMapData data);

      std::string getIptDefaultPackage ();

      void setIptDefaultPackage (std::string package);

      //IPLB methods

      bool isIplbEmpty ();

      bool addIplbEntry (std::string container, std::string product = "-", const char* dn = "");

      void addIplbEntry (LoadModuleMapData data);

      std::string getIplbDefaultPackage ();

      void setIplbDefaultPackage (std::string package);
       
      void removeIplbEntry (std::string container);

      int getIplbSize ();
      
      bool getIplbDefaultPackage (std::string package);
      
      //IPLB methods End

      bool isEpb1Empty ();

      bool addEpb1Entry (std::string container, std::string product = "-", const char * dn = "");

      void addEpb1Entry (LoadModuleMapData data);

      std::string getEpb1DefaultPackage ();

      void setEpb1DefaultPackage (std::string package);

      //	Remove IPT entry from Map
      void removeIptEntry (std::string container);

      //	Remove EPB1 entry from Map
      void removeEpb1Entry (std::string container);

      //	Get FBN basing on the installed package
      std::string getFBN (std::string container);

      int setDefaultInMap (std::string defPackage, std::string fbn);

      int getEpb1Size ();

      int getIptSize ();

      int getScxSize ();
	  
  int getSmxSize ();

      void getProductFromDN(std::string dn, std::string &product_name);

      void getPackageFromDN(std::string dn, std::string &package_name);

      void getDnFromPackage(std::string &dn, std::string container);

      void printMap(std::string fbn);

      void setUsedContainer (std::string container);

      void setNotUsedContainer (std::string container);

      int cleanAllUsedContainers ();

      std::string getCmxDefaultPackage ();

      void setCmxDefaultPackage (std::string package);

      bool isCmxEmpty ();

      int getCmxSize ();

      bool addCmxEntry (std::string container, std::string product = "-", const char* dn = "");

      void addCmxEntry (LoadModuleMapData data);

      void removeCmxEntry (std::string container);

      std::string getEvoEtDefaultPackage ();

      bool getEvoEtDefaultPackage (std::string package);

      void setEvoEtDefaultPackage (std::string package);

      bool isEvoEtEmpty ();

      int getEvoEtSize ();

      bool addEvoEtEntry (std::string container, std::string product = "-", const char* dn = "");

      void addEvoEtEntry (LoadModuleMapData data);

      void removeEvoEtEntry (std::string container);

      void setFirstEvoEtDefaultPackage();

      int CreateBladeSwImmObjects(std::string & dn,std::string xmlpath,int fbn);

      void setFirstScxDefaultPackage ();

      void setFirstSmxDefaultPackage ();

      void setFirstIptDefaultPackage ();

      void setFirstEpb1DefaultPackage ();

      void setFirstCmxDefaultPackage ();

      void setFirstIplbDefaultPackage ();


  public:
    // Additional Public Declarations



  protected:
    // Additional Protected Declarations



  private:

      FIXS_CCH_SoftwareMap();

      std::string pathname;

      FIXS_CCH_SoftwareMap(const FIXS_CCH_SoftwareMap &right);

      virtual ~FIXS_CCH_SoftwareMap();

      ACS_TRA_trace* _trace;

      FIXS_CCH_SoftwareMap & operator=(const FIXS_CCH_SoftwareMap &right);

      int getContainersFromDisk ();

      bool getScxDefaultPackage (std::string package);
	  
	  bool getSmxDefaultPackage (std::string package);

//      void setFirstScxDefaultPackage ();
	  
//	  void setFirstSmxDefaultPackage ();

//      void setFirstIptDefaultPackage ();

//      void setFirstEpb1DefaultPackage ();

      bool getIptDefaultPackage (std::string package);

//      void setFirstIplbDefaultPackage ();

      bool getEpb1DefaultPackage (std::string package);

//      void setFirstCmxDefaultPackage ();

      bool getCmxDefaultPackage (std::string package);

      static FIXS_CCH_SoftwareMap* m_instance;

      bool m_initialized;

      //	key = container package (CXP190xxx_R4A),
      //	value = scx data {containerPackage, softwareVersion, isDefault, isUsed}

      softwareMap_t m_scxMap;

      //	key = container package (CXP190xxx_R4A),
      //	value = scx data {containerPackage, softwareVersion, isDefault, isUsed}

      softwareMap_t m_iptMap;

      //	key = container package (CXP190xxx_R4A),
      //	value = epb1 data {containerPackage, softwareVersion, isDefault, isUsed}

      softwareMap_t m_epb1Map;

      softwareMap_t m_cmxMap;

      softwareMap_t m_evoEtMap;
  
      softwareMap_t m_iplbMap;
	  
	  softwareMap_t m_smxMap;

      std::string m_SCX_defaultPackage;
	  
	  std::string m_SMX_defaultPackage;

      std::string m_IPT_defaultPackage;

      std::string m_EPB1_defaultPackage;

      std::string m_CMX_defaultPackage;

      std::string m_EVOET_defaultPackage;
     
      std::string m_IPLB_defaultPackage;  

      std::string m_nbiSwPath;

      ACS_TRA_Logging *FIXS_CCH_logging;

};
// Class FIXS_CCH_SoftwareMap::LoadModuleMapData
// Class FIXS_CCH_SoftwareMap

#endif
