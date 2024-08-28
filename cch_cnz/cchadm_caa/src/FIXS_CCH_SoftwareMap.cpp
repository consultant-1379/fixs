// FIXS_CCH_SoftwareMap
#include "FIXS_CCH_SoftwareMap.h"
#define MAX_PATH_CCH 1000

namespace
{
   //CRITICAL_SECTION s_cs;
	ACE_thread_mutex_t s_cs;

   const char * const SCXFBN = "SCXB";
   const char * const IPTFBN = "IPTB";
   const char * const EPB1FBN = "EPB1";
   const char * const CETB1FBN = "CETB1";
   const char * const CMXFBN = "CMXB";
   const char * const EVOETFBN = "EVOET";
   const char * const IPLBFBN  = "IPLB";
   const char * const SMXFBN = "SMXB";
   const char * const IN_USE = "IN USE";
   const char * const NOT_USED = "NOT USED";
   const char * const LIST_HEADER = "CONTAINER\t\tPRODUCT\t\tFBN\tCOMMENT";
 //  ACS_TRA_trace traceObj = ACS_TRA_DEF("FIXS_CCH_SoftwareMap", "C512");
}

namespace {
   struct Lock
   {
         Lock() {
            //EnterCriticalSection(&s_cs);
        	 ACE_OS::thread_mutex_trylock(&s_cs);
         };

         ~Lock() {
            // if( std::uncaught_exception() ) std::cout << "leave Lock region while exception active." << std::endl;
           // LeaveCriticalSection(&s_cs);
        	 ACE_OS::thread_mutex_unlock(&s_cs);
         };
   };
}

// Class FIXS_CCH_SoftwareMap::LoadModuleMapData
// Additional Declarations
// Class FIXS_CCH_SoftwareMap


FIXS_CCH_SoftwareMap* FIXS_CCH_SoftwareMap::m_instance = 0;


FIXS_CCH_SoftwareMap::FIXS_CCH_SoftwareMap(): m_initialized(false), m_scxMap(), m_iptMap(), m_epb1Map(), m_cmxMap(), m_evoEtMap(),m_iplbMap(), m_smxMap()
{
	m_SCX_defaultPackage = "";
	m_IPT_defaultPackage = "";
	m_EPB1_defaultPackage = "";
	m_CMX_defaultPackage = "";
	m_EVOET_defaultPackage = "";
        m_IPLB_defaultPackage = "";
	m_SMX_defaultPackage = "";

	_trace = new ACS_TRA_trace("CCH SoftwareMap");

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

}


FIXS_CCH_SoftwareMap::~FIXS_CCH_SoftwareMap()
{
	delete(_trace);

	FIXS_CCH_logging = 0;
}

FIXS_CCH_SoftwareMap* FIXS_CCH_SoftwareMap::getInstance ()
{
	// TR HW52924 fix
	if (m_instance == 0)
	{
		Lock lock;
		if (m_instance == 0)
		{
			m_instance = new FIXS_CCH_SoftwareMap();
		}
	}
	return m_instance;

}

void FIXS_CCH_SoftwareMap::finalize ()
{
	delete(m_instance);
	m_instance = NULL;
}


 int FIXS_CCH_SoftwareMap::initialize ()
{
	Lock lock;

	if (m_initialized) return 0;

	if (FIXS_CCH_SoftwareMap::getContainersFromDisk() != 0) //fill m_scxMap
	{
		// error
		{
			char trace[512] = {0};
			snprintf(trace, sizeof(trace) - 1, "[%s@%d] Error in getContainersFromDisk", __FUNCTION__, __LINE__);
			std::cout << trace << std::endl;
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_ERROR);
		}
		return -1;
	}

	if (m_scxMap.empty() && m_iptMap.empty()&& m_iplbMap.empty() && m_epb1Map.empty() && m_cmxMap.empty() && m_evoEtMap.empty() && m_smxMap.empty())
	{
		// trace - no installation yet
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " WARNING: no installation yet!" << std::endl;
		m_initialized = true;
		return 0;
	}
	else
	{
		if (!m_scxMap.empty())
		{
			// read and set default package into the internal map
			if (getScxDefaultPackage(m_SCX_defaultPackage) == false)
			{
				// error setting default package
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " error setting SCX default package!" << std::endl;
				{
					char trace[512] = {0};
					snprintf(trace, sizeof(trace) - 1, "[%s@%d] error setting SCX default package!", __FUNCTION__, __LINE__);
					std::cout << trace << std::endl;
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_ERROR);
				}

				//set the first software into the map and IMM
				setFirstScxDefaultPackage();
			}

		}
		if (!m_smxMap.empty())
		{
			// read and set default package into the internal map
			if (getSmxDefaultPackage(m_SMX_defaultPackage) == false)
			{
				// error setting default package
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " error setting SMX default package!" << std::endl;
				{
					char trace[512] = {0};
					snprintf(trace, sizeof(trace) - 1, "[%s@%d] error setting SMX default package!", __FUNCTION__, __LINE__);
					std::cout << trace << std::endl;
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_ERROR);
				}

				//set the first software into the map and IMM
				setFirstSmxDefaultPackage();
			}

		}


		if (!m_iplbMap.empty())
		{
			// read and set default package into the internal map
			if (getIplbDefaultPackage(m_IPLB_defaultPackage) == false)
			{
				// error setting default package
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " error setting IPLB default package!" << std::endl;
				{
					char trace[512] = {0};
					snprintf(trace, sizeof(trace) - 1, "[%s@%d] error setting IPLB default package!", __FUNCTION__, __LINE__);
					std::cout << trace << std::endl;
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_ERROR);
				}

				//set the first software into the map and IMM
				setFirstIplbDefaultPackage();
			}
                }



		if (!m_iptMap.empty())
		{
			// read and set default package into the internal map
			if (getIptDefaultPackage(m_IPT_defaultPackage) == false)
			{
				// error setting default package
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " error setting IPT default package!" << std::endl;
				{
					char trace[512] = {0};
					snprintf(trace, sizeof(trace) - 1, "[%s@%d] error setting IPT default package", __FUNCTION__, __LINE__);
					std::cout << trace << std::endl;
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_ERROR);
				}
				//set the first software into the map and IMM
				setFirstIptDefaultPackage();
			}
		}

		if (!m_epb1Map.empty())
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setting EPB1 default package!, epb1 dp: "<< m_EPB1_defaultPackage.c_str() << std::endl;
			// read and set default package into the internal map
			if (getEpb1DefaultPackage(m_EPB1_defaultPackage) == false)
			{
				// error setting default package
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " error setting EPB1 default package!" << std::endl;
				{
					char trace[512] = {0};
					snprintf(trace, sizeof(trace) - 1, "[%s@%d] error setting EPB1 default package", __FUNCTION__, __LINE__);
					std::cout << trace << std::endl;
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_ERROR);
				}
				//set the first software into the map and IMM
				setFirstEpb1DefaultPackage();
			}
		}

		if (!m_cmxMap.empty())
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setting CMXB default package!, epb1 dp: "<< m_CMX_defaultPackage.c_str() << std::endl;
			// read and set default package into the internal map
			if (getCmxDefaultPackage(m_CMX_defaultPackage) == false)
			{
				// error setting default package
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " error setting CMXB default package!" << std::endl;
				{
					char trace[512] = {0};
					snprintf(trace, sizeof(trace) - 1, "[%s@%d] error setting CMXB default package", __FUNCTION__, __LINE__);
					std::cout << trace << std::endl;
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_ERROR);
				}
				//set the first software into the map and IMM
				setFirstCmxDefaultPackage();
			}
		}

		if (!m_evoEtMap.empty())
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setting EVOET default package!, epb1 dp: "<< m_EVOET_defaultPackage.c_str() << std::endl;
			// read and set default package into the internal map
			if (getEvoEtDefaultPackage(m_EVOET_defaultPackage) == false)
			{
				// error setting default package
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " error setting EVOET default package!" << std::endl;
				{
					char trace[512] = {0};
					snprintf(trace, sizeof(trace) - 1, "[%s@%d] error setting EVOET default package", __FUNCTION__, __LINE__);
					std::cout << trace << std::endl;
					if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_ERROR);
				}
				//set the first software into the map and IMM
				setFirstEvoEtDefaultPackage();
			}
		}

		//no error found in initialization steps
		m_initialized = true;

		{
			char trace[512] = {0};
			snprintf(trace, sizeof(trace) - 1, "[%s@%d] no error found in initialization steps", __FUNCTION__, __LINE__);
			std::cout << trace << std::endl;
			/*if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_INFO);*/
		}

		printMap("SCXB");
		printMap("EPB1");
		printMap("CMXB");
		printMap("EVOET");
		printMap("IPTB");
                printMap("IPLB");
		return 0;
	}

}

 bool FIXS_CCH_SoftwareMap::isScxEmpty ()
{
	return m_scxMap.empty();
}

 bool FIXS_CCH_SoftwareMap::addScxEntry (std::string container, std::string product, const char* dn)
{

	 	Lock lock;
	 	softwareMap_t::iterator it = m_scxMap.find(container);
	 	if (it == m_scxMap.end())
	 	{
	 		FIXS_CCH_SoftwareMap::LoadModuleMapData data;
	 		data.m_container = container;
	 		data.m_isDefault = isScxEmpty();
	 		data.dname = dn;
	 		data.m_product = product;
	 		data.m_fbn = SCXFBN;
	 		data.m_isUsed = false;
	 		data.m_comment = NOT_USED;
	 		data.m_isFaulty = false;

	 		if (data.m_isDefault)
	 		{
	 			setScxDefaultPackage(data.m_container);
	 			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setting default package!" << std::endl;
	 		}
	 		m_scxMap.insert(softwareMap_t::value_type(data.m_container, data));
	 		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " new SCX: " << data.m_container << ", " << data.m_product << std::endl;
	 	}
	 	else
	 	{
	 		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " package already exists...?!" << std::endl;
	 	}

	 	return true;
}


 void FIXS_CCH_SoftwareMap::removeScxEntry (std::string container)
 {
	 Lock lock;
	 { //trace
		 char tmpStr[512] = {0};
		 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [SCX] remove container '%s'", __FUNCTION__, __LINE__, container.c_str());
		 std::cout << tmpStr << std::endl;
		 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	 }
	 softwareMap_t::iterator it = m_scxMap.find(container);
	 if (it != m_scxMap.end())
	 {
		 m_scxMap.erase(it);
	 }
	 else
	 {
		 { //trace
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [SCX] container not found: '%s'", __FUNCTION__, __LINE__, container.c_str());
			 std::cout << tmpStr << std::endl;
			 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		 }
	 }
}

 bool FIXS_CCH_SoftwareMap::isContainerInstalled (std::string container)
{

	Lock lock;

	bool result = (m_scxMap.find(container) != m_scxMap.end());
	if (!result) result = (m_iptMap.find(container) != m_iptMap.end());
        if (!result) result = (m_iplbMap.find(container) != m_iplbMap.end());
	if (!result) result = (m_epb1Map.find(container) != m_epb1Map.end());
	if (!result) result = (m_cmxMap.find(container) != m_cmxMap.end());
	if (!result) result = (m_evoEtMap.find(container) != m_evoEtMap.end());
	if (!result) result = (m_smxMap.find(container) != m_smxMap.end());
	return result;

		return false;

}

 int FIXS_CCH_SoftwareMap::getContainersFromDisk ()
{
	int res = IMM_Interface::getContainersFromDisk();
	return res;
}

 std::string FIXS_CCH_SoftwareMap::getScxDefaultPackage ()
{
	 return m_SCX_defaultPackage;
}
std::string FIXS_CCH_SoftwareMap::getSmxDefaultPackage ()
{
	 return m_SMX_defaultPackage;
}
 bool FIXS_CCH_SoftwareMap::getSmxDefaultPackage (std::string package)
{
	if (!package.empty())	 return true;
	else return false;
}


 int FIXS_CCH_SoftwareMap::size ()
{

	Lock lock;
	return (m_scxMap.size() + m_iptMap.size() + m_iplbMap.size()+ m_epb1Map.size() + m_cmxMap.size() + m_evoEtMap.size() + m_smxMap.size());

}


 bool FIXS_CCH_SoftwareMap::isDefault (std::string package)
 {

	 {
		 //SCX
		 softwareMap_t::iterator it = m_scxMap.find(package);
		 if (it != m_scxMap.end())
		 {
			 FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			 return data.m_isDefault;
		 }
	 }

	 { // IPT
		 softwareMap_t::iterator it = m_iptMap.find(package);
		 if (it != m_iptMap.end())
		 {
			 FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			 return data.m_isDefault;
		 }
	 
         }
          
	 { // IPLB
		 softwareMap_t::iterator it = m_iplbMap.find(package);
		 if (it != m_iplbMap.end())
		 {
			 FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			 return data.m_isDefault;
		 }
	 }
         
         { // EPB1
		 softwareMap_t::iterator it = m_epb1Map.find(package);
		 if (it != m_epb1Map.end())
		 {
			 FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			 return data.m_isDefault;
		 }
	 }

	 { // CMX
		 softwareMap_t::iterator it = m_cmxMap.find(package);
		 if (it != m_cmxMap.end())
		 {
			 FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			 return data.m_isDefault;
		 }
	 }

	 { // EVOET
		 softwareMap_t::iterator it = m_evoEtMap.find(package);
		 if (it != m_evoEtMap.end())
		 {
			 FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			 return data.m_isDefault;
		 }
	 }

	 {
		 //SMX
		 softwareMap_t::iterator it = m_smxMap.find(package);
		 if (it != m_smxMap.end())
		 {
			 FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			 return data.m_isDefault;
		 }
	 }


	 //package not found
	 return false;
}

 void FIXS_CCH_SoftwareMap::setScxDefaultPackage (std::string package)
{
	 m_SCX_defaultPackage = package;

}

void FIXS_CCH_SoftwareMap::setSmxDefaultPackage (std::string package)
{
	 m_SMX_defaultPackage = package;

}
 bool FIXS_CCH_SoftwareMap::getScxDefaultPackage (std::string package)
{
	if (!package.empty())	 return true;
	else return false;
}

 void FIXS_CCH_SoftwareMap::setFirstScxDefaultPackage()
 {
	 Lock lock;
	 for (softwareMap_t::iterator it = m_scxMap.begin(); it != m_scxMap.end(); it++)
	 {
		 FIXS_CCH_SoftwareMap::LoadModuleMapData scx = (*it).second;

		 setScxDefaultPackage(scx.m_container);

		 //set into the model new default package
		 if (IMM_Interface::setDefaultPackage(scx.dname,1))
		 {
			 cout << "IMM: SET NEW SCX DEFAULT PACKAGE: " << scx.m_container << endl;
			 {
				 char trace[512] = {0};
				 snprintf(trace, sizeof(trace) - 1, "[%s@%d] IMM: SET NEW SCX DEFAULT PACKAGE !", __FUNCTION__, __LINE__);
				 std::cout << trace << std::endl;
				 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_DEBUG);
			 }
			 break;
		 }
	 }


}

 void FIXS_CCH_SoftwareMap::setFirstSmxDefaultPackage()
 {
	 Lock lock;
	 for (softwareMap_t::iterator it = m_smxMap.begin(); it != m_smxMap.end(); it++)
	 {
		 FIXS_CCH_SoftwareMap::LoadModuleMapData smx = (*it).second;

		 setSmxDefaultPackage(smx.m_container);

		 //set into the model new default package
		 if (IMM_Interface::setDefaultPackage(smx.dname,1))
		 {
			 cout << "IMM: SET NEW SMX DEFAULT PACKAGE: " << smx.m_container << endl;
			 {
				 char trace[512] = {0};
				 snprintf(trace, sizeof(trace) - 1, "[%s@%d] IMM: SET NEW SMX DEFAULT PACKAGE !", __FUNCTION__, __LINE__);
				 std::cout << trace << std::endl;
				 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_DEBUG);
			 }
			 break;
		 }
	 }


}


 bool FIXS_CCH_SoftwareMap::isIplbEmpty ()
{
	return m_iplbMap.empty();
}

 bool FIXS_CCH_SoftwareMap::addIplbEntry (std::string container, std::string product, const char* dn)
{

	 	Lock lock;
	 	softwareMap_t::iterator it = m_iplbMap.find(container);
	 	if (it == m_iplbMap.end())
	 	{
	 		FIXS_CCH_SoftwareMap::LoadModuleMapData data;
	 		data.m_container = container;
	 		data.m_isDefault = isIplbEmpty();
	 		data.dname = dn;
	 		data.m_product = product;
	 		data.m_fbn = IPLBFBN;
	 		data.m_isUsed = false;
	 		data.m_comment = NOT_USED;
	 		data.m_isFaulty = false;

	 		if (data.m_isDefault)
	 		{
	 			setIplbDefaultPackage(data.m_container);
	 			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setting default package!" << std::endl;
	 		}
	 		m_iplbMap.insert(softwareMap_t::value_type(data.m_container, data));
	 		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " new IPLB: " << data.m_container << ", " << data.m_product << std::endl;
	 	}
	 	else
	 	{
	 		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " package already exists...?!" << std::endl;
	 	}

	 	return true;
}


 void FIXS_CCH_SoftwareMap::removeIplbEntry (std::string container)
 {
	 Lock lock;
	 { //trace
		 char tmpStr[512] = {0};
		 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [IPLB] remove container '%s'", __FUNCTION__, __LINE__, container.c_str());
		 std::cout << tmpStr << std::endl;
		 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	 }
	 softwareMap_t::iterator it = m_iplbMap.find(container);
	 if (it != m_iplbMap.end())
	 {
		 m_iplbMap.erase(it);
	 }
	 else
	 {
		 { //trace
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [IPLB] container not found: '%s'", __FUNCTION__, __LINE__, container.c_str());
			 std::cout << tmpStr << std::endl;
			 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		 }
	 }

}
void FIXS_CCH_SoftwareMap::setIplbDefaultPackage (std::string package)
{
	 m_IPLB_defaultPackage = package;

}

 std::string FIXS_CCH_SoftwareMap::getIplbDefaultPackage ()
{
	 return m_IPLB_defaultPackage;
}

 bool FIXS_CCH_SoftwareMap::getIplbDefaultPackage (std::string package)
{
	if (!package.empty())	 return true;
	else return false;
}

 void FIXS_CCH_SoftwareMap::setFirstIplbDefaultPackage()
 {
	 Lock lock;
	 for (softwareMap_t::iterator it = m_iplbMap.begin(); it != m_iplbMap.end(); it++)
	 {
		 FIXS_CCH_SoftwareMap::LoadModuleMapData iplb = (*it).second;

		 setIplbDefaultPackage(iplb.m_container);

		 //set into the model new default package
		 if (IMM_Interface::setDefaultPackage(iplb.dname,1))
		 {
			 cout << "IMM: SET NEW IPLB DEFAULT PACKAGE: " << iplb.m_container << endl;
			 {
				 char trace[512] = {0};
				 snprintf(trace, sizeof(trace) - 1, "[%s@%d] IMM: SET NEW IPLB DEFAULT PACKAGE !", __FUNCTION__, __LINE__);
				 std::cout << trace << std::endl;
				 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_DEBUG);
			 }
			 break;
		 }
	 }

}
void FIXS_CCH_SoftwareMap::setFirstIptDefaultPackage()
{
	 Lock lock;
	 for (softwareMap_t::iterator it = m_iptMap.begin(); it != m_iptMap.end(); it++)
	 {
		 FIXS_CCH_SoftwareMap::LoadModuleMapData ipt = (*it).second;

		 setIptDefaultPackage(ipt.m_container);

		 //set into the model new default package
		 if (IMM_Interface::setDefaultPackage(ipt.dname,1))
		 {
			 cout << "IMM: SET NEW IPT DEFAULT PACKAGE..." << endl;
			 {
				 char trace[512] = {0};
				 snprintf(trace, sizeof(trace) - 1, "[%s@%d] IMM: SET NEW IPT DEFAULT PACKAGE !", __FUNCTION__, __LINE__);
				 std::cout << trace << std::endl;
				 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_DEBUG);
			 }
			 break;
		 }


	 }

}

void FIXS_CCH_SoftwareMap::setFirstEpb1DefaultPackage()
{
	 Lock lock;
	 for (softwareMap_t::iterator it = m_epb1Map.begin(); it != m_epb1Map.end(); it++)
	 {
		 FIXS_CCH_SoftwareMap::LoadModuleMapData epb1 = (*it).second;

		 setEpb1DefaultPackage(epb1.m_container);

		 //set into the model new default package
		 if (IMM_Interface::setDefaultPackage(epb1.dname,1))
		 {
			 cout << "IMM: SET NEW EPB1 DEFAULT PACKAGE..." << endl;
			 {
				 char trace[512] = {0};
				 snprintf(trace, sizeof(trace) - 1, "[%s@%d] IMM: SET NEW EPB1 DEFAULT PACKAGE !", __FUNCTION__, __LINE__);
				 std::cout << trace << std::endl;
				 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_DEBUG);
			 }
			 break;
		 }
	 }

}

 std::string FIXS_CCH_SoftwareMap::getSoftwareProduct (std::string &container)
{

	Lock lock;
	bool found = false;
	std::string result;

	{ // SCX
		softwareMap_t::iterator it = m_scxMap.find(container);
		if (it != m_scxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData scx = (*it).second;
			result = scx.m_product;
			found = true;
		}
	}

	if (!found)
	{ // IPT
		softwareMap_t::iterator it = m_iptMap.find(container);
		if (it != m_iptMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData ipt = (*it).second;
			result = ipt.m_product;
			found = true;
		}
	}
        
	if (!found)
	{ // IPLB
		softwareMap_t::iterator it = m_iplbMap.find(container);
		if (it != m_iplbMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData iplb = (*it).second;
			result = iplb.m_product;
			found = true;
		}
        } 

	if (!found)
	{ // EPB1
		softwareMap_t::iterator it = m_epb1Map.find(container);
		if (it != m_epb1Map.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData epb = (*it).second;
			result = epb.m_product;
			found = true;
		}
	}

	if (!found)
	{ // CMXB
		softwareMap_t::iterator it = m_cmxMap.find(container);
		if (it != m_cmxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData cmx = (*it).second;
			result = cmx.m_product;
			found = true;
		}
	}

	if (!found)
	{ // EVOET
		softwareMap_t::iterator it = m_evoEtMap.find(container);
		if (it != m_evoEtMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData evoEt = (*it).second;
			result = evoEt.m_product;
			found = true;
		}
	}
	if(!found)
	{ // SMX
		softwareMap_t::iterator it = m_smxMap.find(container);
		if (it != m_smxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData smx = (*it).second;
			result = smx.m_product;
			found = true;
		}	
	}
	if (_trace->ACS_TRA_ON())
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] result: %s  -  container: %s ", __FUNCTION__, __LINE__,result.c_str(), container.c_str());
		_trace->ACS_TRA_event(1, tmpStr);

	}
	return result;
}

 bool FIXS_CCH_SoftwareMap::isProductInstalled (std::string cxc, int fbn)
{

	Lock lock;
	if(fbn == CCH_Util::SCXB)
	{
		for (softwareMap_t::iterator it = m_scxMap.begin(); it != m_scxMap.end(); it++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
			{
				return true;
			}
		}
	}
	else if(fbn == CCH_Util::IPTB)
	{
		for (softwareMap_t::iterator it = m_iptMap.begin(); it != m_iptMap.end(); it++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
			{
				return true;
			}
		}
	
        }
	else if(fbn == CCH_Util::IPLB)
	{
		for (softwareMap_t::iterator it = m_iplbMap.begin(); it != m_iplbMap.end(); it++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
			cout << "data.m_product.c_str() " << data.m_product.c_str() << " cxc.c_str() " <<  cxc.c_str() << endl;
			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
			{
				return true;
			}
		}
	
	

        }
	else if(fbn == CCH_Util::EPB1)
	{
		for (softwareMap_t::iterator it = m_epb1Map.begin(); it != m_epb1Map.end(); it++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
			{
				return true;
			}

		}
	}
	else if(fbn == CCH_Util::CMXB)
	{
		for (softwareMap_t::iterator it = m_cmxMap.begin(); it != m_cmxMap.end(); it++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
			{
				return true;
			}

		}
	}
	else if(fbn == CCH_Util::EVOET)
	{
		for (softwareMap_t::iterator it = m_evoEtMap.begin(); it != m_evoEtMap.end(); it++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
			{
				return true;
			}

		}
	}
	else if(fbn == CCH_Util::SMXB)
	{
		for (softwareMap_t::iterator it = m_smxMap.begin(); it != m_smxMap.end(); it++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
			{
				return true;
			}
		}
	}

	return false;
}

 bool FIXS_CCH_SoftwareMap::isProductInstalled (std::string cxc)
 {

 	Lock lock;

 		for (softwareMap_t::iterator it = m_scxMap.begin(); it != m_scxMap.end(); it++)
 		{
 			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
 			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
 			{
 				return true;
 			}
 		}


 		for (softwareMap_t::iterator it = m_iptMap.begin(); it != m_iptMap.end(); it++)
 		{
 			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
 			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
 			{
 				return true;
 			}
 		}
                
 		for (softwareMap_t::iterator it = m_iplbMap.begin(); it != m_iplbMap.end(); it++)
 		{
 			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
 			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
 			{
 				return true;
 			}
                }

 		for (softwareMap_t::iterator it = m_epb1Map.begin(); it != m_epb1Map.end(); it++)
 		{
 			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
 			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
 			{
 				return true;
 			}

 		}

 		for (softwareMap_t::iterator it = m_cmxMap.begin(); it != m_cmxMap.end(); it++)
 		{
 			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
 			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
 			{
 				return true;
 			}

 		}

 		for (softwareMap_t::iterator it = m_evoEtMap.begin(); it != m_evoEtMap.end(); it++)
 		{
 			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
 			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
 			{
 				return true;
 			}

 		}
		
 		for (softwareMap_t::iterator it = m_smxMap.begin(); it != m_smxMap.end(); it++)
 		{
 			FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
 			if ((strcmp(data.m_product.c_str(), cxc.c_str()) == 0))
 			{
 				return true;
 			}

 		}
 		return false;
 }

 std::string FIXS_CCH_SoftwareMap::getContainerByProduct (std::string &product)
{

	//Lock lock;
	std::string container;

	for (softwareMap_t::iterator it = m_scxMap.begin(); it != m_scxMap.end(); it++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
		if ((strcmp(data.m_product.c_str(), product.c_str()) == 0))
		{
			container = data.m_container;
			return container;
		}
	}
	for (softwareMap_t::iterator it = m_iptMap.begin(); it != m_iptMap.end(); it++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
		if ((strcmp(data.m_product.c_str(), product.c_str()) == 0))
		{
			container = data.m_container;
			return container;
		}
	}
	for (softwareMap_t::iterator it = m_epb1Map.begin(); it != m_epb1Map.end(); it++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
		if ((strcmp(data.m_product.c_str(), product.c_str()) == 0))
		{
			container = data.m_container;
			return container;
		}
	}

	for (softwareMap_t::iterator it = m_cmxMap.begin(); it != m_cmxMap.end(); it++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
		if ((strcmp(data.m_product.c_str(), product.c_str()) == 0))
		{
			container = data.m_container;
			return container;
		}
	}

        
	for (softwareMap_t::iterator it = m_iplbMap.begin(); it != m_iplbMap.end(); it++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
		if ((strcmp(data.m_product.c_str(), product.c_str()) == 0))
		{
			container = data.m_container;
			return container;
		}
        }

	for (softwareMap_t::iterator it = m_evoEtMap.begin(); it != m_evoEtMap.end(); it++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
		if ((strcmp(data.m_product.c_str(), product.c_str()) == 0))
		{
			container = data.m_container;
			return container;
		}
	}

	for (softwareMap_t::iterator it = m_smxMap.begin(); it != m_smxMap.end(); it++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = it->second;
		if ((strcmp(data.m_product.c_str(), product.c_str()) == 0))
		{
			container = data.m_container;
			return container;
		}
	}
	return container;

}

 bool FIXS_CCH_SoftwareMap::isUsedPackage (std::string package)
{

//	Lock lock;

	{ // SCX
		softwareMap_t::iterator it = m_scxMap.find(package);
		if (it != m_scxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			if (data.m_comment.compare(IN_USE) == 0)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
	}

	{ // IPT
		softwareMap_t::iterator it = m_iptMap.find(package);
		if (it != m_iptMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			if (data.m_comment.compare(IN_USE) == 0)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
	}

	{ // EPB1
		softwareMap_t::iterator it = m_epb1Map.find(package);
		if (it != m_epb1Map.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			if (data.m_comment.compare(IN_USE) == 0)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
	}
        
	{ // IPLB
		softwareMap_t::iterator it = m_iplbMap.find(package);
		if (it != m_iplbMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			if (data.m_comment.compare(IN_USE) == 0)
			{
				return true;
			}
                }
        } 
	{ // CMXB
		softwareMap_t::iterator it = m_cmxMap.find(package);
		if (it != m_cmxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			if (data.m_comment.compare(IN_USE) == 0)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
	}
	{ // EVOET
		softwareMap_t::iterator it = m_evoEtMap.find(package);
		if (it != m_evoEtMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			if (data.m_comment.compare(IN_USE) == 0)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
	}
	{ // SMX
		softwareMap_t::iterator it = m_smxMap.find(package);
		if (it != m_smxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			if (data.m_comment.compare(IN_USE) == 0)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
	}

	//package not found
	return false;

}

 bool FIXS_CCH_SoftwareMap::isIptEmpty ()
{

	return m_iptMap.empty();

}

 bool FIXS_CCH_SoftwareMap::addIptEntry (std::string container, std::string product, const char* dn)
{

	Lock lock;
	softwareMap_t::iterator it = m_iptMap.find(container);
	if (it == m_iptMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data;
		data.m_container = container;
		data.m_isDefault = isIptEmpty();
		data.m_product = product;
		data.dname = dn;
		data.m_fbn = IPTFBN;
		data.m_comment = NOT_USED;
		data.m_isUsed = false;
		data.m_isFaulty = false;

		if (data.m_isDefault)
		{
			setIptDefaultPackage(data.m_container);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setting default package into CS!" << std::endl;
		}
		m_iptMap.insert(softwareMap_t::value_type(data.m_container, data));
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " new IPT: " << data.m_container << ", " << data.m_product << std::endl;
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " package already exists...?!" << std::endl;
	}

	return true;

}

 std::string FIXS_CCH_SoftwareMap::getIptDefaultPackage ()
{
	 return m_IPT_defaultPackage;
}

bool FIXS_CCH_SoftwareMap::getIptDefaultPackage (std::string package)
 {
	if (package != "")	 return true;
		else return false;
 }

 void FIXS_CCH_SoftwareMap::setIptDefaultPackage (std::string package)
{
	 m_IPT_defaultPackage = package;
}

 bool FIXS_CCH_SoftwareMap::isEpb1Empty ()
{

	return m_epb1Map.empty();

}

 bool FIXS_CCH_SoftwareMap::addEpb1Entry (std::string container, std::string product, const char * dn)
{

	Lock lock;
	softwareMap_t::iterator it = m_epb1Map.find(container);

	if (it == m_epb1Map.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data;
		data.m_container = container;
		data.m_isDefault = isEpb1Empty();
		data.m_product = product;
		data.dname = dn;
		data.m_fbn = EPB1FBN;
		data.m_isUsed = false;
		data.m_comment = NOT_USED;
		data.m_isFaulty = false;

		if (data.m_isDefault)
		{
			setEpb1DefaultPackage(data.m_container);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setting default package!" << std::endl;
		}

		m_epb1Map.insert(softwareMap_t::value_type(data.m_container, data));
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " package already exists...?!" << std::endl;
	}

	return true;
}

 bool FIXS_CCH_SoftwareMap::getEpb1DefaultPackage (std::string package)
{
	 if (package != "")	 return true;
	 	else return false;
}

 std::string FIXS_CCH_SoftwareMap::getEpb1DefaultPackage ()
{

	 return m_EPB1_defaultPackage;

}

 void FIXS_CCH_SoftwareMap::setEpb1DefaultPackage (std::string package)
{
	 m_EPB1_defaultPackage = package;
}

 void FIXS_CCH_SoftwareMap::removeIptEntry (std::string container)
{

	Lock lock;
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [IPT] remove container '%s'", __FUNCTION__, __LINE__, container.c_str());
		std::cout << tmpStr << std::endl;
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}
	softwareMap_t::iterator it = m_iptMap.find(container);
	if (it != m_iptMap.end())
	{
		m_iptMap.erase(it);
	}
	else
	{
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [IPT] container not found: '%s'", __FUNCTION__, __LINE__, container.c_str());
			std::cout << tmpStr << std::endl;
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}
	}

}

 void FIXS_CCH_SoftwareMap::removeEpb1Entry (std::string container)
{

	Lock lock;
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [EPB1] remove container '%s'", __FUNCTION__, __LINE__, container.c_str());
		std::cout << tmpStr << std::endl;
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}
	softwareMap_t::iterator it = m_epb1Map.find(container);
	if (it != m_epb1Map.end())
	{
		m_epb1Map.erase(it);
	}
	else
	{
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [EPB1] container not found: '%s'", __FUNCTION__, __LINE__, container.c_str());
			std::cout << tmpStr << std::endl;
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}
	}

}

 std::string FIXS_CCH_SoftwareMap::getFBN (std::string container)
{

	Lock lock;
	std::cout << __FUNCTION__ <<__LINE__ << " container: " << container<< std::endl;

	{ // SCX
		softwareMap_t::iterator it = m_scxMap.find(container);
		if (it != m_scxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_fbn;
		}
	}

	{ // IPT
		softwareMap_t::iterator it = m_iptMap.find(container);
		if (it != m_iptMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_fbn;
		}
	}

	{ // EPB1
		softwareMap_t::iterator it = m_epb1Map.find(container);
		if (it != m_epb1Map.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_fbn;
		}
	}
        
	{ //IPLB 
		softwareMap_t::iterator it = m_iplbMap.find(container);
		if (it != m_iplbMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_fbn;
		}
        }
	{ // CMXB
		softwareMap_t::iterator it = m_cmxMap.find(container);
		if (it != m_cmxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_fbn;
		}
	}

	{ // EVOET
		softwareMap_t::iterator it = m_evoEtMap.find(container);
		if (it != m_evoEtMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_fbn;
		}
	}
	{ // SMX
		softwareMap_t::iterator it = m_smxMap.find(container);
		if (it != m_smxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_fbn;
		}
	}
	//container not found
	return "";


}

 int FIXS_CCH_SoftwareMap::setDefaultInMap (std::string defPackage, std::string fbn)
{
	 { //trace
		 char tmpStr[512] = {0};
		 snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH_SoftwareMap - set new [%s] default package : [%s]",fbn.c_str(), defPackage.c_str());
		 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	 }

	if (fbn == "SCXB")
	{
		softwareMap_t::iterator map_it = m_scxMap.find(defPackage);
		if (map_it != m_scxMap.end())
		{
			for (softwareMap_t::iterator itr = m_scxMap.begin(); itr != m_scxMap.end(); itr++)
			{
				(*itr).second.m_isDefault = false;
			}

			((*map_it).second).m_isDefault = true;

			setScxDefaultPackage((*map_it).second.m_container);
			return 0;
		}
		else
		{
			for (softwareMap_t::iterator itr = m_scxMap.begin(); itr != m_scxMap.end(); itr++)
			{
				if((*itr).second.m_isDefault == true)
				{
					setScxDefaultPackage((*itr).second.m_container);
					break;
				}
			}
			return -1; //package is not present in scx software map
		}
	}
	else if (fbn == "IPTB")
	{
		softwareMap_t::iterator map_it = m_iptMap.find(defPackage);
		if (map_it != m_iptMap.end())
		{
			for (softwareMap_t::iterator itr = m_iptMap.begin(); itr != m_iptMap.end(); itr++)
			{
				(*itr).second.m_isDefault = false;
			}
			((*map_it).second).m_isDefault = true;
			setIptDefaultPackage((*map_it).second.m_container);
			return 0;
		}
		else
		{
			for (softwareMap_t::iterator itr = m_iptMap.begin(); itr != m_iptMap.end(); itr++)
			{
				if((*itr).second.m_isDefault == true)
				{
					setIptDefaultPackage((*itr).second.m_container);
					break;
				}
			}
			return -1; //package is not present in ipt software map
		}
	}
	else if (fbn == "EPB1")
	{
		softwareMap_t::iterator map_it = m_epb1Map.find(defPackage);
		if (map_it != m_epb1Map.end())
		{
			for (softwareMap_t::iterator itr = m_epb1Map.begin(); itr != m_epb1Map.end(); itr++)
			{
				(*itr).second.m_isDefault = false;
			}
			((*map_it).second).m_isDefault = true;
			setEpb1DefaultPackage((*map_it).second.m_container);
			return 0;
		}
		else
		{
			for (softwareMap_t::iterator itr = m_epb1Map.begin(); itr != m_epb1Map.end(); itr++)
			{
				if((*itr).second.m_isDefault == true)
				{
					setEpb1DefaultPackage((*itr).second.m_container);
					break;
				}
			}
			return -1; //package is not present in epb1 software map
		}
	}
	else if (fbn == "CMXB")
	{
		softwareMap_t::iterator map_it = m_cmxMap.find(defPackage);
		if (map_it != m_cmxMap.end())
		{
			for (softwareMap_t::iterator itr = m_cmxMap.begin(); itr != m_cmxMap.end(); itr++)
			{
				(*itr).second.m_isDefault = false;
			}
			((*map_it).second).m_isDefault = true;
			setCmxDefaultPackage((*map_it).second.m_container);
			return 0;
		}
		else
		{
			for (softwareMap_t::iterator itr = m_cmxMap.begin(); itr != m_cmxMap.end(); itr++)
			{
				if((*itr).second.m_isDefault == true)
				{
					setCmxDefaultPackage((*itr).second.m_container);
					break;
				}
			}
			return -1; //package is not present in epb1 software map
		}
	}

	else if (fbn == "IPLB")
	{
		softwareMap_t::iterator map_it = m_iplbMap.find(defPackage);
		if (map_it != m_iplbMap.end())
		{
			for (softwareMap_t::iterator itr = m_iplbMap.begin(); itr != m_iplbMap.end(); itr++)
			{
				(*itr).second.m_isDefault = false;
			}
			((*map_it).second).m_isDefault = true;
			setIplbDefaultPackage((*map_it).second.m_container);
			return 0;
		}
		else
		{
			for (softwareMap_t::iterator itr = m_iplbMap.begin(); itr != m_iplbMap.end(); itr++)
			{
				if((*itr).second.m_isDefault == true)
				{
					setIplbDefaultPackage((*itr).second.m_container);
					break;
				}
			}
			return -1; //package is not present in IPLB software map
		}
	}
	else if (fbn == "EVOET")
	{
		softwareMap_t::iterator map_it = m_evoEtMap.find(defPackage);
		if (map_it != m_evoEtMap.end())
		{
			for (softwareMap_t::iterator itr = m_evoEtMap.begin(); itr != m_evoEtMap.end(); itr++)
			{
				(*itr).second.m_isDefault = false;
			}
			((*map_it).second).m_isDefault = true;
			setEvoEtDefaultPackage((*map_it).second.m_container);
   			return 0;
		}
		else
		{
			for (softwareMap_t::iterator itr = m_evoEtMap.begin(); itr != m_evoEtMap.end(); itr++)
			{
				if((*itr).second.m_isDefault == true)
				{
					setEvoEtDefaultPackage((*itr).second.m_container);
					break;
				}
			}
			return -1; //package is not present in epb2 software map
		}
	}
	else if (fbn == "SMXB")
	{
		softwareMap_t::iterator map_it = m_smxMap.find(defPackage);
		if (map_it != m_smxMap.end())
		{
			for (softwareMap_t::iterator itr = m_smxMap.begin(); itr != m_smxMap.end(); itr++)
			{
				(*itr).second.m_isDefault = false;
			}

			((*map_it).second).m_isDefault = true;

			setSmxDefaultPackage((*map_it).second.m_container);
			return 0;
		}
		else
		{
			for (softwareMap_t::iterator itr = m_smxMap.begin(); itr != m_smxMap.end(); itr++)
			{
				if((*itr).second.m_isDefault == true)
				{
					setSmxDefaultPackage((*itr).second.m_container);
					break;
				}
			}
			return -1; //package is not present in smx software map
		}
	}
	return -1;

}

int FIXS_CCH_SoftwareMap::getEpb1Size ()
{

	Lock lock;
	return m_epb1Map.size();

}

int FIXS_CCH_SoftwareMap::getIptSize ()
{

Lock lock;
return m_iptMap.size();

}

int FIXS_CCH_SoftwareMap::getIplbSize ()
{

Lock lock;
return m_iplbMap.size();

}

int FIXS_CCH_SoftwareMap::getScxSize ()
{

Lock lock;
return m_scxMap.size();

}


int FIXS_CCH_SoftwareMap::getSmxSize ()
{

Lock lock;
return m_smxMap.size();

}

void FIXS_CCH_SoftwareMap::getProductFromDN(std::string dn, std::string &product_name)
{
	//Lock lock;
	softwareMap_t::iterator it_scx = m_scxMap.find(dn);
	if (it_scx != m_scxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_scx).second;
		product_name = data.m_product;
		return;

	}

	softwareMap_t::iterator it_epb1 = m_epb1Map.find(dn);
	if (it_epb1 != m_epb1Map.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_epb1).second;
		product_name = data.m_product;
		return;

	}

	softwareMap_t::iterator it_ipt = m_iptMap.find(dn);
	if (it_ipt != m_iptMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_ipt).second;
		product_name = data.m_product;
		return;

	}

	softwareMap_t::iterator it_cmx = m_cmxMap.find(dn);
	if (it_cmx != m_cmxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_cmx).second;
		product_name = data.m_product;
		return;

	}

	softwareMap_t::iterator it_evoEt = m_evoEtMap.find(dn);
	if (it_evoEt != m_evoEtMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_evoEt).second;
		product_name = data.m_product;
		return;

	}
       
	softwareMap_t::iterator it_iplb = m_iplbMap.find(dn);
	if (it_iplb != m_iplbMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_iplb).second;
		product_name = data.m_product;
		return;

	}
	softwareMap_t::iterator it_smx = m_smxMap.find(dn);
	if (it_smx != m_smxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_smx).second;
		product_name = data.m_product;
		return;

	}
}

void FIXS_CCH_SoftwareMap::getPackageFromDN(std::string dn, std::string &package_name)
{
	//Lock lock;
	softwareMap_t::iterator it_scx = m_scxMap.find(dn);
	if (it_scx != m_scxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_scx).second;
		package_name = data.m_container;
		return;

	}

	softwareMap_t::iterator it_epb1 = m_epb1Map.find(dn);
	if (it_epb1 != m_epb1Map.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_epb1).second;
		package_name = data.m_container;
		return;

	}

	softwareMap_t::iterator it_ipt = m_iptMap.find(dn);
	if (it_ipt != m_iptMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_ipt).second;
		package_name = data.m_container;
		return;

	}

	softwareMap_t::iterator it_cmx = m_cmxMap.find(dn);
	if (it_cmx != m_cmxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_cmx).second;
		package_name = data.m_container;
		return;

	}

	softwareMap_t::iterator it_iplb = m_iplbMap.find(dn);
	if (it_iplb != m_iplbMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_iplb).second;
		package_name = data.m_container;
		return;

	}
	softwareMap_t::iterator it_evoEt = m_evoEtMap.find(dn);
	if (it_evoEt != m_evoEtMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_evoEt).second;
		package_name = data.m_container;
		return;

	}
	softwareMap_t::iterator it_smx = m_smxMap.find(dn);
	if (it_smx != m_smxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_smx).second;
		package_name = data.m_container;
		return;

	}
}

void FIXS_CCH_SoftwareMap::getDnFromPackage(std::string &dn, std::string container)
{
	//Lock lock;
	softwareMap_t::iterator it_scx = m_scxMap.find(container);
	if (it_scx != m_scxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_scx).second;
		dn = data.dname;
		return;
	}

	softwareMap_t::iterator it_epb1 = m_epb1Map.find(container);
	if (it_epb1 != m_epb1Map.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_epb1).second;
		dn = data.dname;
		return;
	}

	softwareMap_t::iterator it_ipt = m_iptMap.find(container);
	if (it_ipt != m_iptMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_ipt).second;
		dn = data.dname;
		return;
	}

	softwareMap_t::iterator it_cmx = m_cmxMap.find(container);
	if (it_cmx != m_cmxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_cmx).second;
		dn = data.dname;
		return;
	}

	softwareMap_t::iterator it_iplb = m_iplbMap.find(container);
	if (it_iplb != m_iplbMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_iplb).second;
		dn = data.dname;
		return;
	}
	softwareMap_t::iterator it_evoEt = m_evoEtMap.find(container);
	if (it_evoEt != m_evoEtMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_evoEt).second;
		dn = data.dname;
		return;
	}
	softwareMap_t::iterator it_smx = m_smxMap.find(container);
	if (it_smx != m_smxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_smx).second;
		dn = data.dname;
		return;
	}
}

void FIXS_CCH_SoftwareMap::printMap(std::string fbn)
{
	if (fbn.compare("SCXB") == 0)
	{
		cout << "----------- software SCXB map ---------- " << endl;

		for (softwareMap_t::iterator itr = m_scxMap.begin(); itr != m_scxMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;
			cout << "container: "<< data.m_container.c_str() << " is default: " << data.m_isDefault << " comment: " << data.m_comment.c_str() << " " << data.m_isUsed << " faulty: " << data.m_isFaulty << endl;
		}
	}
	else if (fbn.compare("EPB1") == 0)
	{
		cout << "----------- software EPB1 map ---------- " << endl;

		for (softwareMap_t::iterator itr = m_epb1Map.begin(); itr != m_epb1Map.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;
			cout << "container: "<< data.m_container.c_str() << " is default: " << data.m_isDefault << " comment: " << data.m_comment.c_str() << " " << data.m_isUsed << " faulty: " << data.m_isFaulty << endl;
		}
	}
	else if (fbn.compare("IPTB") == 0)
	{
		cout << "----------- software IPTB map ---------- " << endl;

		for (softwareMap_t::iterator itr = m_iptMap.begin(); itr != m_iptMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;
			cout << "container: "<< data.m_container.c_str() << " is default: " << data.m_isDefault << " comment: " << data.m_comment.c_str() << " " << data.m_isUsed << " faulty: " << data.m_isFaulty << endl;
		}
	}
        
	else if (fbn.compare("IPLB") == 0)
	{
		cout << "----------- software IPTB map ---------- " << endl;

		for (softwareMap_t::iterator itr = m_iplbMap.begin(); itr != m_iplbMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;
			cout << "container: "<< data.m_container.c_str() << " is default: " << data.m_isDefault << " comment: " << data.m_comment.c_str() << " " << data.m_isUsed << " faulty: " << data.m_isFaulty << endl;
                }
        }
	else if (fbn.compare("CMXB") == 0)
	{
		cout << "----------- software CMXB map ---------- " << endl;

		for (softwareMap_t::iterator itr = m_cmxMap.begin(); itr != m_cmxMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;
			cout << "container: "<< data.m_container.c_str() << " is default: " << data.m_isDefault << " comment: " << data.m_comment.c_str() << " " << data.m_isUsed << " faulty: " << data.m_isFaulty << endl;
		}
	}
	else if (fbn.compare("EVOET") == 0)
	{
		cout << "----------- software EVOET map ---------- " << endl;

		for (softwareMap_t::iterator itr = m_evoEtMap.begin(); itr != m_evoEtMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;
			cout << "container: "<< data.m_container.c_str() << " is default: " << data.m_isDefault << " comment: " << data.m_comment.c_str() << " " << data.m_isUsed << " faulty: " << data.m_isFaulty << endl;
		}
	}
	else if (fbn.compare("SMXB") == 0)
	{
		cout << "----------- software SMXB map ---------- " << endl;

		for (softwareMap_t::iterator itr = m_smxMap.begin(); itr != m_smxMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;
			cout << "container: "<< data.m_container.c_str() << " is default: " << data.m_isDefault << " comment: " << data.m_comment.c_str() << " " << data.m_isUsed << " faulty: " << data.m_isFaulty << endl;
		}
	}
}

void FIXS_CCH_SoftwareMap::addScxEntry (LoadModuleMapData data)
{
	m_scxMap.insert(softwareMap_t::value_type(data.m_container, data));
}

void FIXS_CCH_SoftwareMap::addIptEntry (LoadModuleMapData data)
{
	m_iptMap.insert(softwareMap_t::value_type(data.m_container, data));
}

void FIXS_CCH_SoftwareMap::addIplbEntry (LoadModuleMapData data)
{
	m_iplbMap.insert(softwareMap_t::value_type(data.m_container, data));
}

void FIXS_CCH_SoftwareMap::addEpb1Entry (LoadModuleMapData data)
{
	m_epb1Map.insert(softwareMap_t::value_type(data.m_container, data));
}

void FIXS_CCH_SoftwareMap::setUsedContainer (std::string container)
{
	softwareMap_t::iterator it_scx = m_scxMap.find(container);
	if (it_scx != m_scxMap.end())
	{
		if (((*it_scx).second).m_isUsed == false)
		{
			((*it_scx).second).m_comment = IN_USE;
			((*it_scx).second).m_isUsed = true;

			return;
		}
	}

	softwareMap_t::iterator it_epb1 = m_epb1Map.find(container);
	if (it_epb1 != m_epb1Map.end())
	{
		if (((*it_epb1).second).m_isUsed == false)
		{
			((*it_epb1).second).m_comment = IN_USE;
			((*it_epb1).second).m_isUsed = true;

			return;
		}

	}

	softwareMap_t::iterator it_ipt = m_iptMap.find(container);
	if (it_ipt != m_iptMap.end())
	{
		if (((*it_ipt).second).m_isUsed == false)
		{
			((*it_ipt).second).m_comment = IN_USE;
			((*it_ipt).second).m_isUsed = true;

			return;
		}

	}

	softwareMap_t::iterator it_cmx = m_cmxMap.find(container);
	if (it_cmx != m_cmxMap.end())
	{
		if (((*it_cmx).second).m_isUsed == false)
		{
			((*it_cmx).second).m_comment = IN_USE;
			((*it_cmx).second).m_isUsed = true;

			return;
		}

	}

	softwareMap_t::iterator it_iplb = m_iplbMap.find(container);
	if (it_iplb != m_iplbMap.end())
	{
		if (((*it_iplb).second).m_isUsed == false)
		{
			((*it_iplb).second).m_comment = IN_USE;
			((*it_iplb).second).m_isUsed = true;

			return;
		}

	}

	softwareMap_t::iterator it_evoEt = m_evoEtMap.find(container);
	if (it_evoEt != m_evoEtMap.end())
	{
		if (((*it_evoEt).second).m_isUsed == false)
		{
			((*it_evoEt).second).m_comment = IN_USE;
			((*it_evoEt).second).m_isUsed = true;

			return;
		}

	}
	softwareMap_t::iterator it_smx = m_smxMap.find(container);
	if (it_smx != m_smxMap.end())
	{
		if (((*it_smx).second).m_isUsed == false)
		{
			((*it_smx).second).m_comment = IN_USE;
			((*it_smx).second).m_isUsed = true;

			return;
		}
	}
}

void FIXS_CCH_SoftwareMap::setNotUsedContainer (std::string container)
{
	softwareMap_t::iterator it_scx = m_scxMap.find(container);
	if (it_scx != m_scxMap.end())
	{
		if (((*it_scx).second).m_isUsed == true)
		{
			((*it_scx).second).m_comment = NOT_USED;
			((*it_scx).second).m_isUsed = false;

			return;
		}
	}

	softwareMap_t::iterator it_iplb = m_iplbMap.find(container);
	if (it_iplb != m_iplbMap.end())
	{
		if (((*it_iplb).second).m_isUsed == true)
		{
			((*it_iplb).second).m_comment = NOT_USED;
			((*it_iplb).second).m_isUsed = false;

			return;
		}
	}

	softwareMap_t::iterator it_epb1 = m_epb1Map.find(container);
	if (it_epb1 != m_epb1Map.end())
	{
		if (((*it_epb1).second).m_isUsed == true)
		{
			((*it_epb1).second).m_comment = NOT_USED;
			((*it_epb1).second).m_isUsed = false;

			return;
		}

	}

	softwareMap_t::iterator it_ipt = m_iptMap.find(container);
	if (it_ipt != m_iptMap.end())
	{
		if (((*it_ipt).second).m_isUsed == true)
		{
			((*it_ipt).second).m_comment = NOT_USED;
			((*it_ipt).second).m_isUsed = false;

			return;
		}

	}

	softwareMap_t::iterator it_cmx = m_cmxMap.find(container);
	if (it_cmx != m_cmxMap.end())
	{
		if (((*it_cmx).second).m_isUsed == true)
		{
			((*it_cmx).second).m_comment = NOT_USED;
			((*it_cmx).second).m_isUsed = false;

			return;
		}

	}

	softwareMap_t::iterator it_evoEt = m_evoEtMap.find(container);
	if (it_evoEt != m_evoEtMap.end())
	{
		if (((*it_evoEt).second).m_isUsed == true)
		{
			((*it_evoEt).second).m_comment = NOT_USED;
			((*it_evoEt).second).m_isUsed = false;

			return;
		}

	}
	softwareMap_t::iterator it_smx = m_smxMap.find(container);
	if (it_smx != m_smxMap.end())
	{
		if (((*it_smx).second).m_isUsed == true)
		{
			((*it_smx).second).m_comment = NOT_USED;
			((*it_smx).second).m_isUsed = false;

			return;
		}
	}

}

int FIXS_CCH_SoftwareMap::cleanAllUsedContainers ()
{
	int res = 0;

	//////////////////////////////////////////////////////////////
	//					SET NOT USED PACKAGES					//
	//////////////////////////////////////////////////////////////

	//loop SCX software
	for (softwareMap_t::iterator itr = m_scxMap.begin(); itr != m_scxMap.end(); itr++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

		//set not used in IMM and internal map
		IMM_Interface::setEmptyUsedPackage(data.m_product);
	}

        //loop IPLB software 
	for (softwareMap_t::iterator itr = m_iplbMap.begin(); itr != m_iplbMap.end(); itr++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

		//set not used in IMM and internal map
		IMM_Interface::setEmptyUsedPackage(data.m_product);
	}

	//loop EPB1 software
	for (softwareMap_t::iterator itr = m_epb1Map.begin(); itr != m_epb1Map.end(); itr++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

		//set not used in IMM and internal map
		IMM_Interface::setEmptyUsedPackage(data.m_product);
	}

	//loop IPTB software
	for (softwareMap_t::iterator itr = m_iptMap.begin(); itr != m_iptMap.end(); itr++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

		//set not used in IMM and internal map
		IMM_Interface::setEmptyUsedPackage(data.m_product);
	}

	//loop CMXB software
	for (softwareMap_t::iterator itr = m_cmxMap.begin(); itr != m_cmxMap.end(); itr++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

		//set not used in IMM and internal map
		IMM_Interface::setEmptyUsedPackage(data.m_product);
	}

	//loop EVOET software
	for (softwareMap_t::iterator itr = m_evoEtMap.begin(); itr != m_evoEtMap.end(); itr++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

		//set not used in IMM and internal map
		IMM_Interface::setEmptyUsedPackage(data.m_product);
	}

	//loop SMX software
	for (softwareMap_t::iterator itr = m_smxMap.begin(); itr != m_smxMap.end(); itr++)
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

		//set not used in IMM and internal map
		IMM_Interface::setEmptyUsedPackage(data.m_product);
	}
	return res;
}

bool FIXS_CCH_SoftwareMap::isFaultyPackage (std::string package)
{

//	Lock lock;

	{ // SCX
		softwareMap_t::iterator it = m_scxMap.find(package);
		if (it != m_scxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_isFaulty;
		}
	}
        
	{ // IPLB
		softwareMap_t::iterator it = m_iplbMap.find(package);
		if (it != m_iplbMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_isFaulty;
		}
	}


	{ // IPT
		softwareMap_t::iterator it = m_iptMap.find(package);
		if (it != m_iptMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_isFaulty;
		}
	}

	{ // EPB1
		softwareMap_t::iterator it = m_epb1Map.find(package);
		if (it != m_epb1Map.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_isFaulty;
		}
	}

	{ // CMXB
		softwareMap_t::iterator it = m_cmxMap.find(package);
		if (it != m_cmxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_isFaulty;
		}
	}

	{ // EVOET
		softwareMap_t::iterator it = m_evoEtMap.find(package);
		if (it != m_evoEtMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_isFaulty;
		}
	}

	{ // SMX
		softwareMap_t::iterator it = m_smxMap.find(package);
		if (it != m_smxMap.end())
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it).second;
			return data.m_isFaulty;
		}
	}
	//package not found
	return false;

}


bool FIXS_CCH_SoftwareMap::checkFaultyContainers (std::string fbn)
{
	bool res = false;

	if (strcmp(fbn.c_str(),"SCXB")==0)
	{
		//loop SCX software
		for (softwareMap_t::iterator itr = m_scxMap.begin(); itr != m_scxMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

			if (data.m_isFaulty == true)
				return true;

		}
	}

	if (strcmp(fbn.c_str(),"IPLB")==0)
	{
		//loop IPLB software
		for (softwareMap_t::iterator itr = m_iplbMap.begin(); itr != m_iplbMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

			if (data.m_isFaulty == true)
				return true;

		}
	}

	if (strcmp(fbn.c_str(),"EPB1")==0)
	{
		//loop SCX software
		for (softwareMap_t::iterator itr = m_epb1Map.begin(); itr != m_epb1Map.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

			if (data.m_isFaulty == true)
				return true;

		}
	}
	if (strcmp(fbn.c_str(),"IPTB")==0)
	{
		//loop SCX software
		for (softwareMap_t::iterator itr = m_iptMap.begin(); itr != m_iptMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

			if (data.m_isFaulty == true)
				return true;

		}
	}
	if (strcmp(fbn.c_str(),"CMXB")==0)
	{
		//loop CMX software
		for (softwareMap_t::iterator itr = m_cmxMap.begin(); itr != m_cmxMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

			if (data.m_isFaulty == true)
				return true;

		}
	}
	if (strcmp(fbn.c_str(),"EVOET")==0)
	{
		//loop EVOET software
		for (softwareMap_t::iterator itr = m_evoEtMap.begin(); itr != m_evoEtMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

			if (data.m_isFaulty == true)
				return true;

		}
	}

	if (strcmp(fbn.c_str(),"SMXB")==0)
	{
		//loop SMX software
		for (softwareMap_t::iterator itr = m_smxMap.begin(); itr != m_smxMap.end(); itr++)
		{
			FIXS_CCH_SoftwareMap::LoadModuleMapData data = itr->second;

			if (data.m_isFaulty == true)
				return true;

		}
	}

	return res;
}

void FIXS_CCH_SoftwareMap::setNotFaultyContainer (std::string container)
{
	softwareMap_t::iterator it_scx = m_scxMap.find(container);
	if (it_scx != m_scxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_scx).second;
		if (data.m_isFaulty == true)
		{
			(*it_scx).second.m_isFaulty = false;
			return;
		}
	}

	softwareMap_t::iterator it_iplb = m_iplbMap.find(container);
	if (it_iplb != m_iplbMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_iplb).second;
		if (data.m_isFaulty == true)
		{
			(*it_iplb).second.m_isFaulty = false;
			return;
		}
	}

	softwareMap_t::iterator it_epb1 = m_epb1Map.find(container);
	if (it_epb1 != m_epb1Map.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_epb1).second;
		if (data.m_isFaulty == true)
		{
			(*it_epb1).second.m_isFaulty = false;
			return;
		}

	}

	softwareMap_t::iterator it_ipt = m_iptMap.find(container);
	if (it_ipt != m_iptMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_ipt).second;
		if (data.m_isFaulty == true)
		{
			(*it_ipt).second.m_isFaulty = false;
			return;
		}

	}

	softwareMap_t::iterator it_cmx = m_cmxMap.find(container);
	if (it_cmx != m_cmxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_cmx).second;
		if (data.m_isFaulty == true)
		{
			(*it_cmx).second.m_isFaulty = false;
			return;
		}

	}

	softwareMap_t::iterator it_evoEt = m_evoEtMap.find(container);
	if (it_evoEt != m_evoEtMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_evoEt).second;
		if (data.m_isFaulty == true)
		{
			(*it_evoEt).second.m_isFaulty = false;
			return;
		}

	}
	softwareMap_t::iterator it_smx = m_smxMap.find(container);
	if (it_smx != m_smxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data = (*it_smx).second;
		if (data.m_isFaulty == true)
		{
			(*it_smx).second.m_isFaulty = false;
			return;
		}
	}
}

bool FIXS_CCH_SoftwareMap::getCmxDefaultPackage (std::string package)
 {
	 if (package != "")	 return true;
	 else return false;
 }

 std::string FIXS_CCH_SoftwareMap::getCmxDefaultPackage ()
 {

	 return m_CMX_defaultPackage;

 }

 void FIXS_CCH_SoftwareMap::setCmxDefaultPackage (std::string package)
 {
	 m_CMX_defaultPackage = package;
 }


void FIXS_CCH_SoftwareMap::setFirstCmxDefaultPackage()
{
	 Lock lock;
	 for (softwareMap_t::iterator it = m_cmxMap.begin(); it != m_cmxMap.end(); it++)
	 {
		 FIXS_CCH_SoftwareMap::LoadModuleMapData cmx = (*it).second;

		 setCmxDefaultPackage(cmx.m_container);

		 //set into the model new default package
		 if (IMM_Interface::setDefaultPackage(cmx.dname,1))
		 {
			 cout << "IMM: SET NEW CMXB DEFAULT PACKAGE..." << endl;
			 {
				 char trace[512] = {0};
				 snprintf(trace, sizeof(trace) - 1, "[%s@%d] IMM: SET NEW CMXB DEFAULT PACKAGE !", __FUNCTION__, __LINE__);
				 std::cout << trace << std::endl;
				 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_DEBUG);
			 }
			 break;
		 }
	 }

}


bool FIXS_CCH_SoftwareMap::isCmxEmpty (){

	return m_cmxMap.empty();
}

int FIXS_CCH_SoftwareMap::getCmxSize (){

	Lock lock;
	return m_cmxMap.size();
}

bool FIXS_CCH_SoftwareMap::addCmxEntry (std::string container, std::string product, const char* dn){
	Lock lock;
	softwareMap_t::iterator it = m_cmxMap.find(container);
	if (it == m_cmxMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data;
		data.m_container = container;
		data.m_isDefault = isCmxEmpty();
		data.dname = dn;
		data.m_product = product;
		data.m_fbn = CMXFBN;
		data.m_isUsed = false;
		data.m_comment = NOT_USED;
		data.m_isFaulty = false;

		if (data.m_isDefault)
		{
			setCmxDefaultPackage(data.m_container);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setting default package!" << std::endl;
		}
		m_cmxMap.insert(softwareMap_t::value_type(data.m_container, data));
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " new CMX: " << data.m_container << ", " << data.m_product << std::endl;
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " package already exists...?!" << std::endl;
	}

	return true;
}

void FIXS_CCH_SoftwareMap::addCmxEntry (LoadModuleMapData data){

	m_cmxMap.insert(softwareMap_t::value_type(data.m_container, data));
}

void FIXS_CCH_SoftwareMap::removeCmxEntry (std::string container){

	Lock lock;
	{ //trace
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [CMX] remove container '%s'", __FUNCTION__, __LINE__, container.c_str());
		std::cout << tmpStr << std::endl;
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	}
	softwareMap_t::iterator it = m_cmxMap.find(container);
	if (it != m_cmxMap.end())
	{
		m_cmxMap.erase(it);
	}
	else
	{
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [CMX] container not found: '%s'", __FUNCTION__, __LINE__, container.c_str());
			std::cout << tmpStr << std::endl;
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		}
	}
}

bool FIXS_CCH_SoftwareMap::isEvoEtEmpty ()
{

	return m_evoEtMap.empty();

}

 bool FIXS_CCH_SoftwareMap::addEvoEtEntry (std::string container, std::string product, const char * dn)
{

	Lock lock;
	softwareMap_t::iterator it = m_evoEtMap.find(container);

	if (it == m_evoEtMap.end())
	{
		FIXS_CCH_SoftwareMap::LoadModuleMapData data;
		data.m_container = container;
		data.m_isDefault = isEvoEtEmpty();
		data.m_product = product;
		data.dname = dn;
		data.m_fbn = EVOETFBN;
		data.m_isUsed = false;
		data.m_comment = NOT_USED;
		data.m_isFaulty = false;

		if (data.m_isDefault)
		{
			setEvoEtDefaultPackage(data.m_container);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setting default package!" << std::endl;
		}

		m_evoEtMap.insert(softwareMap_t::value_type(data.m_container, data));
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " package already exists...?!" << std::endl;
	}

	return true;
}

 bool FIXS_CCH_SoftwareMap::getEvoEtDefaultPackage (std::string package)
{
	 if (package != "")	 return true;
	 	else return false;
}

 std::string FIXS_CCH_SoftwareMap::getEvoEtDefaultPackage ()
{

	 return m_EVOET_defaultPackage;

}

 void FIXS_CCH_SoftwareMap::setEvoEtDefaultPackage (std::string package)
 {
	 m_EVOET_defaultPackage = package;
 }

 void FIXS_CCH_SoftwareMap::removeEvoEtEntry (std::string container)
 {

	 Lock lock;
	 { //trace
		 char tmpStr[512] = {0};
		 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [EVOET] remove container '%s'", __FUNCTION__, __LINE__, container.c_str());
		 std::cout << tmpStr << std::endl;
		 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	 }
	 softwareMap_t::iterator it = m_evoEtMap.find(container);
	 if (it != m_evoEtMap.end())
	 {
		 m_evoEtMap.erase(it);
	 }
	 else
	 {
		 { //trace
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [EVOET] container not found: '%s'", __FUNCTION__, __LINE__, container.c_str());
			 std::cout << tmpStr << std::endl;
			 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		 }
	 }

 }

 int FIXS_CCH_SoftwareMap::getEvoEtSize ()
 {

 	Lock lock;
 	return m_evoEtMap.size();

 }

 void FIXS_CCH_SoftwareMap::setFirstEvoEtDefaultPackage()
 {
 	 Lock lock;
 	 for (softwareMap_t::iterator it = m_evoEtMap.begin(); it != m_evoEtMap.end(); it++)
 	 {
 		 FIXS_CCH_SoftwareMap::LoadModuleMapData evoEt = (*it).second;

 		 setEvoEtDefaultPackage(evoEt.m_container);

 		 //set into the model new default package
 		 if (IMM_Interface::setDefaultPackage(evoEt.dname,1))
 		 {
 			 cout << "IMM: SET NEW EVOET DEFAULT PACKAGE..." << endl;
 			 {
 				 char trace[512] = {0};
 				 snprintf(trace, sizeof(trace) - 1, "[%s@%d] IMM: SET NEW EVOET DEFAULT PACKAGE !", __FUNCTION__, __LINE__);
 				 std::cout << trace << std::endl;
 				 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, trace);
 				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(trace,LOG_LEVEL_DEBUG);
 			 }
 			 break;
 		 }
 	 }

 }

 void FIXS_CCH_SoftwareMap::addEvoEtEntry (LoadModuleMapData data)
 {
	 m_evoEtMap.insert(softwareMap_t::value_type(data.m_container, data));
 }

 bool FIXS_CCH_SoftwareMap::isSmxEmpty ()
{
	return m_smxMap.empty();
}
 bool FIXS_CCH_SoftwareMap::addSmxEntry (std::string container, std::string product, const char* dn)
{

	 	Lock lock;
	 	softwareMap_t::iterator it = m_smxMap.find(container);
	 	if (it == m_smxMap.end())
	 	{
	 		FIXS_CCH_SoftwareMap::LoadModuleMapData data;
	 		data.m_container = container;
	 		data.m_isDefault = isSmxEmpty();
	 		data.dname = dn;
	 		data.m_product = product;
	 		data.m_fbn = SMXFBN;
	 		data.m_isUsed = false;
	 		data.m_comment = NOT_USED;
	 		data.m_isFaulty = false;

	 		if (data.m_isDefault)
	 		{
	 			setSmxDefaultPackage(data.m_container);
	 			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " setting default package!" << std::endl;
	 		}
	 		m_smxMap.insert(softwareMap_t::value_type(data.m_container, data));
	 		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " new SMX: " << data.m_container << ", " << data.m_product << std::endl;
	 	}
	 	else
	 	{
	 		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " package already exists...?!" << std::endl;
	 	}

	 	return true;
}

void FIXS_CCH_SoftwareMap::addSmxEntry (LoadModuleMapData data)
{
	m_smxMap.insert(softwareMap_t::value_type(data.m_container, data));
}

 void FIXS_CCH_SoftwareMap::removeSmxEntry (std::string container)
 {
	 Lock lock;
	 { //trace
		 char tmpStr[512] = {0};
		 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [SMX] remove container '%s'", __FUNCTION__, __LINE__, container.c_str());
		 std::cout << tmpStr << std::endl;
		 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
	 }
	 softwareMap_t::iterator it = m_smxMap.find(container);
	 if (it != m_smxMap.end())
	 {
		 m_smxMap.erase(it);
	 }
	 else
	 {
		 { //trace
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] [SMX] container not found: '%s'", __FUNCTION__, __LINE__, container.c_str());
			 std::cout << tmpStr << std::endl;
			 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
		 }
	 }
}
int FIXS_CCH_SoftwareMap::CreateBladeSwImmObjects(std::string & dn,std::string xmlPath, int fbn /*,std::string containerName*/)
{
	int resp=0;

	char * dn_swRep=(char*)malloc(200);
	memset(dn_swRep,0,200);

	char * container=(char*)malloc(100);
	memset(container,0,100); 

	FIXS_CCH_Installer installer;
	if(!installer.SetXmlFileAfterRestore(xmlPath))
	{
		resp=1;
		return resp;
	}
	resp = installer.fetchPackageInfo();
	if(resp!=0)
	{
		return resp;
	}
	int isDefault = 0;
	int swVerType = 0;

	// TR HW71825 BEGIN

	if ( fbn == IMM_Util::CMXB )
	{
		swVerType =  installer.getCmxVersionType();
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] cmx version type '%d'", __FUNCTION__, __LINE__, swVerType);
		std::cout << tmpStr << std::endl;
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_WARN);
	}

	// TR HW71825 END
	std::string installedProduct = installer.getInstalledProduct();
	std::string installedPackage = installer.getInstalledPackage();
	std::string fileNamePackage = installer.getFileNameInPackage();
	std::string swPath = xmlPath + "/" + fileNamePackage;
	if(fbn != IMM_Util::EVOET && fbn !=IMM_Util::EPB1)
		sprintf(container,"%s=%s",IMM_Util::ATT_APSWPACK_RDN,installedPackage.c_str());
	else
		sprintf(container,"%s=%s",IMM_Util::ATT_CPSWPACK_RDN,installedPackage.c_str());
	sprintf(dn_swRep,"%s,%s",container,IMM_Util::RDN_BLADESWM);
	dn=dn_swRep;
	//sprintf(dn,"%s",dn_swRep);
	EventHandle m_shutdownEvent=0;
	const int WAIT_TIMEOUT= -2;
	const int WAIT_OBJECT_0=0;
	const int Shutdown_Event_Index=0;
	const int Number_Of_Alarm_Events=1;
	DWORD Safe_Timeout = 1000;
	bool res=false;
	bool m_exit= false;
	bool respValue=false;
	int count=0;

	if(fbn != IMM_Util::EVOET && fbn !=IMM_Util::EPB1)
		respValue=IMM_Interface::createApSwObject(container,isDefault,fbn,swPath,installedProduct, swVerType);
	else
		respValue=IMM_Interface::createCpSwObject(container,isDefault,fbn,swPath,installedProduct);
	if(respValue)
	{
		std::string id("");
		if(!IMM_Interface::createSwManagedActionResultObject(dn_swRep,installedPackage,id)){
			resp =1;
			free(container);
			free(dn_swRep);
			return resp;
		}
		else
			IMM_Interface::setSwManagedActionReferenceStruct(dn_swRep,id);
	}
	if (m_shutdownEvent == 0)
		m_shutdownEvent=FIXS_CCH_Event::OpenNamedEvent(CCH_Util::EVENT_NAME_SHUTDOWN);
	EventHandle handleArr=m_shutdownEvent;

	while (!m_exit && !respValue) // Set to true when the thread shall exit
	{
		if(count >= 60){
			m_exit=true;
			break;
		}
		//wait for 1s
		int returnValue = FIXS_CCH_Event::WaitForEvents(Number_Of_Alarm_Events, &handleArr, Safe_Timeout);

		switch (returnValue)
		{
		case WAIT_TIMEOUT:  // Time out
		{
			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Waiting for events...", __FUNCTION__, __LINE__);
				_trace->ACS_TRA_event(1,tmpStr);
			}
			if(fbn != IMM_Util::EVOET && fbn !=IMM_Util::EPB1)
				res=IMM_Interface::createApSwObject(container,isDefault,fbn,swPath,installedProduct, swVerType);
			else
				res=IMM_Interface::createCpSwObject(container,isDefault,fbn,swPath,installedProduct);
			if(res)
			{
				resp=0;
				std::string id("");
				if(!IMM_Interface::createSwManagedActionResultObject(dn_swRep,installedPackage,id))
				{
					IMM_Util::deleteImmObject(dn_swRep,ACS_APGCC_SUBTREE);	
					resp=1;
					count++;
				}
				else {
					IMM_Interface::setSwManagedActionReferenceStruct(dn_swRep,id);
					m_exit = true;
					respValue=true;
				}
			}
			else {
				resp=1;
				count++;
			}
			break;
		}
		case (WAIT_OBJECT_0 + Shutdown_Event_Index):  // Shutdown event
					{
			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Recieved a shutdown event", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				_trace->ACS_TRA_event(1,tmpStr);
			}
			m_exit = true;      // Terminate the thread
			count++;
			break;
					}
		default:
		{
			if (_trace->ACS_TRA_ON())
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Error: Unexpected event signaled.", __FUNCTION__, __LINE__);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				_trace->ACS_TRA_event(1,tmpStr);
			}
			count++;
			break;
		}
		} // End of switch
	}//while

	free(container);
	free(dn_swRep);
	return resp;
}
