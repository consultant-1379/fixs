#ifndef FIXS_CCH_FSMDataStorage_h
#define FIXS_CCH_FSMDataStorage_h 1

#include <string>
#include <vector>
#include "FIXS_CCH_Util.h"
#include <iostream>
#include "FIXS_CCH_SoftwareMap.h"
#include "acs_apgcc_omhandler.h"



class FIXS_CCH_FSMDataStorage 
{
  //## begin FIXS_CCH_FSMDataStorage%4C25AE1802BD.initialDeclarations preserve=yes
  //## end FIXS_CCH_FSMDataStorage%4C25AE1802BD.initialDeclarations

  public:

    //## Other Operations (specified)
      //## Operation: getInstance%4C25C63B038F
      static  FIXS_CCH_FSMDataStorage * getInstance ();

      //## Operation: saveUpgradeStatus%4C285C0400C3
      bool saveUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::SWData data);

      //## Operation: saveUpgradeStatus%4C285D9E025B
      bool saveUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::PFMData data);

      //## Operation: saveUpgradeStatus%4C285D9F022C
      bool saveUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::IpmiData data);

      //## Operation: readUpgradeStatus%4C28C39C0343
      bool readUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::SWData &data);

      //## Operation: readUpgradeStatus%4C28C3BE0363
      bool readUpgradeStatus (std::string dn, unsigned short slot, CCH_Util::PFMData &data);

      //## Operation: readUpgradeStatus%4C28C3D801AE
      bool readUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::IpmiData &data);

      //## Operation: deleteStateMachineSection%4C7CFEC8005A
      //bool deleteStateMachineSection (unsigned long magazine, unsigned short slot, ACS_CS_API_HWC_NS::HWC_FBN_Identifier fbn);

      //## Operation: saveUpgradeStatus%4D663CD5006F
      bool saveUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::BoardSWData data);

      //## Operation: readUpgradeStatus%4D663D0802D1
      bool readUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::BoardSWData &data);

      bool readUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::FwData &data);

      bool readUpgradeStatus (unsigned long magazine, unsigned short slot, CCH_Util::fwRemoteData &data);

      bool saveUpgradeStatus (std::string dn, CCH_Util::FwData &data);

      bool saveFwShFileName (std::string dn, std::string valueToSet);

      bool readFwShFileName (std::string dn, std::string &value);

      bool saveFwXmlFileName (std::string dn, std::string valueToSet);

      bool readFwXmlFileName (std::string dn, std::string &value);

      bool saveFwRevisionState (std::string dn, std::string valueToSet);

      bool readFwRevisionState (std::string dn, std::string &value);

      //## Operation: getSections%4DD55CA803D5
      void getSections (std::vector<std::string> &names);

      bool createFwStorageEnvironment(std::string dn, CCH_Util::FwData &data);

      bool saveRemoteFWUpgradeStatus(std::string dn, CCH_Util::FwData &data);

      bool saveFwUpgradeStatusFromFile(std::string dn, CCH_Util::fwRemoteData &data, bool isSMX);

      bool saveOldFwUpgradeStatus(std::string dn, CCH_Util::fwRemoteData &data);
    // Additional Public Declarations
      //## begin FIXS_CCH_FSMDataStorage%4C25AE1802BD.public preserve=yes
      //## end FIXS_CCH_FSMDataStorage%4C25AE1802BD.public

  protected:
    // Additional Protected Declarations
      //## begin FIXS_CCH_FSMDataStorage%4C25AE1802BD.protected preserve=yes
      //## end FIXS_CCH_FSMDataStorage%4C25AE1802BD.protected

  private:
    //## Constructors (generated)
      FIXS_CCH_FSMDataStorage();

      FIXS_CCH_FSMDataStorage(const FIXS_CCH_FSMDataStorage &right);

    //## Assignment Operation (generated)
      FIXS_CCH_FSMDataStorage & operator=(const FIXS_CCH_FSMDataStorage &right);


    //## Other Operations (specified)
      //## Operation: createFolder%4C28A8F501BD
      bool createFolder (std::string folder);

      //## Operation: write%4C25C78A01DF
      bool write (std::string section, std::string key, std::string value);

      //## Operation: read%4C25C8B10347
      bool read (std::string section, std::string key, std::string &value);

      //## Operation: prepareForWriting%4C2895B50369
      bool prepareForWriting ();

      //## Operation: commitChanges%4C2895E80148
      bool commitChanges ();

    // Data Members for Class Attributes

      //## Attribute: instance%4C25C5DB0094
      //## begin FIXS_CCH_FSMDataStorage::instance%4C25C5DB0094.attr preserve=no  private: static FIXS_CCH_FSMDataStorage* {UA} 0
      static FIXS_CCH_FSMDataStorage* s_instance;
      //## end FIXS_CCH_FSMDataStorage::instance%4C25C5DB0094.attr

    // Additional Private Declarations
      //## begin FIXS_CCH_FSMDataStorage%4C25AE1802BD.private preserve=yes
      //## end FIXS_CCH_FSMDataStorage%4C25AE1802BD.private

  private: //## implementation
    // Additional Implementation Declarations
      //## begin FIXS_CCH_FSMDataStorage%4C25AE1802BD.implementation preserve=yes
      //## end FIXS_CCH_FSMDataStorage%4C25AE1802BD.implementation

};

//## begin FIXS_CCH_FSMDataStorage%4C25AE1802BD.postscript preserve=yes
//## end FIXS_CCH_FSMDataStorage%4C25AE1802BD.postscript

// Class FIXS_CCH_FSMDataStorage 

//## begin module%4C25AE1802BD.epilog preserve=yes
//## end module%4C25AE1802BD.epilog


#endif
