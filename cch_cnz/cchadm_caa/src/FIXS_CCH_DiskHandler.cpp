#include <iostream>
#include <algorithm>
#include <vector>

// FIXS_CCH_DiskHandler
#include "FIXS_CCH_DiskHandler.h"

namespace
{

}

// Class FIXS_CCH_DiskHandler
std::string FIXS_CCH_DiskHandler::m_scxFolder;

std::string FIXS_CCH_DiskHandler::m_iplbFolder;

std::string FIXS_CCH_DiskHandler::m_iptFolder;

std::string FIXS_CCH_DiskHandler::m_tftpRootFolder;

std::string FIXS_CCH_DiskHandler::m_tftpRootFolderDefault;

std::string FIXS_CCH_DiskHandler::m_epb1Folder;

std::string FIXS_CCH_DiskHandler::m_cmxFolder;

std::string FIXS_CCH_DiskHandler::m_evoEtFolder;

std::string FIXS_CCH_DiskHandler::m_apzFolder;

std::string FIXS_CCH_DiskHandler::m_ipmiupgFolder;

std::string FIXS_CCH_DiskHandler::m_smxFolder;

CCH_Util::EnvironmentType FIXS_CCH_DiskHandler::m_environment;

ACS_TRA_Logging* FIXS_CCH_DiskHandler::FIXS_CCH_logging;

bool FIXS_CCH_DiskHandler::startAfterRestore;

FIXS_CCH_DiskHandler::~FIXS_CCH_DiskHandler()
{
	closeLogInstance();
}

bool FIXS_CCH_DiskHandler::deleteTempFiles()
{
	bool res = true;

	if (CCH_Util::removeFolder(CCH_Util::TEMP_FOLDER.c_str()) != 0)
	{
		char tmpStr[512] = { 0 };
		snprintf(tmpStr, sizeof(tmpStr) - 1,
				"[%s] removeFolder failed with error: %d", __FUNCTION__, -1);
		std::cout << tmpStr << std::endl;
		res = false;
	}

	return res;
}

void FIXS_CCH_DiskHandler::deleteFile(const char *file)
{
	if (CCH_Util::PathFileExists(file))
	{
		remove(file);
	}
}

void FIXS_CCH_DiskHandler::createDefaultFolder(
		CCH_Util::EnvironmentType environment)
{
	bool createScxFolder, createEpb1Folder, createEvoETFolder, createIptFolder,
			createCmxFolder, createIplbFolder, createIpmiupgFolder, createSmxFolder;

	ACS_TRA_trace* acs_trace = new ACS_TRA_trace("FIXS_CCH_DiskHandler");
	FIXS_CCH_logging = getLogInstance();

	m_apzFolder = "/data/apz/data";
	m_tftpRootFolderDefault = m_apzFolder;

	createScxFolder = false;
	createEpb1Folder = false;
	createEvoETFolder = false;
	createIptFolder = false;
	createCmxFolder = false;
	createIplbFolder = false;
	createIpmiupgFolder = false;
	createSmxFolder = false;
	//get APZ folder
	switch (environment)
	{
	case CCH_Util::MULTIPLECP_CBA:
	case CCH_Util::MULTIPLECP_NOTCBA:
	case CCH_Util::MULTIPLECP_DMX:
	case CCH_Util::SINGLECP_DMX:	
	case CCH_Util::MULTIPLECP_SMX:
		m_tftpRootFolder = m_apzFolder;
		break;
	case CCH_Util::SINGLECP_CBA:
	case CCH_Util::SINGLECP_NOTCBA:
	case CCH_Util::SINGLECP_SMX:
	default:
		m_tftpRootFolder = m_apzFolder + "/boot";
		break;
	}

	m_scxFolder = m_tftpRootFolder + "/SCX/";
	m_epb1Folder = m_tftpRootFolder + "/EPB1/";
	m_iptFolder = "/data/IPT/DATA/";
	m_cmxFolder = m_tftpRootFolder + "/CMX/";
	m_iplbFolder = "/data/ext/IPLB/DATA/";
	m_evoEtFolder = m_tftpRootFolder + "/EVOET/";
	m_smxFolder = m_tftpRootFolder + "/SMX/";
	m_ipmiupgFolder = m_apzFolder + "/boot"; 
	
	switch (environment)
	{
	case CCH_Util::SINGLECP_CBA:
	case CCH_Util::MULTIPLECP_CBA:
		createScxFolder = true;
		createEpb1Folder = true;
		createEvoETFolder = true;
		createIptFolder = true;
		createCmxFolder = true;
		break;
	case CCH_Util::SINGLECP_NOTCBA:
	case CCH_Util::SINGLECP_DMX:
		createScxFolder = false;
		createEpb1Folder = false;
		createEvoETFolder = false;
		createIptFolder = false;
		createCmxFolder = false;
		createIpmiupgFolder = true;
		break;
	case CCH_Util::MULTIPLECP_DMX:
		createScxFolder = false;
		createEpb1Folder = false;
		createEvoETFolder = false;
		createIptFolder = false;
		createIplbFolder = true;
		createIpmiupgFolder = true;
		createCmxFolder = false;
		break;
	case CCH_Util::MULTIPLECP_SMX:
	case CCH_Util::SINGLECP_SMX:
		createEpb1Folder = true;
		createEvoETFolder = true;
		createIptFolder = true;//To check if required in SMX environment
		createSmxFolder = true;		
		break;
	case CCH_Util::MULTIPLECP_NOTCBA:
	default:
		createScxFolder = false;
		createEpb1Folder = false;
		createEvoETFolder = false;
		createIptFolder = true;
		createCmxFolder = false;
		break;
	}

	int retry = 0;
	while (ACS_APGCC::is_active_node() == false)
	{
		{
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"FIXS_CCH - is not active node yet...");
			FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
			if (acs_trace->ACS_TRA_ON())
				acs_trace->ACS_TRA_event(1, tmpStr);
		}
		sleep(5);
		if (++retry > 3)
			break;
	}

	if (createScxFolder)
	{
		createScxDefaultDirectory(acs_trace);
	}

	if (createEpb1Folder)
	{
		createEpblDefaultDirectory(acs_trace);
	}

	if (createIptFolder)
	{
		createIptDefaultDirectory(acs_trace);
	}

	if (createCmxFolder)
	{
		createCmxDefaultDirectory(acs_trace);
	}

	if (createIpmiupgFolder)
	{
		createIpmiupgDefaultDirectory(acs_trace);
	}

	if (createIplbFolder)
	{
		createIplbDefaultDirectory(acs_trace);
	}

	if (createEvoETFolder)
	{
		createEvoEtDefaultDirectory(acs_trace);
	}
	if (createSmxFolder)
	{
		createSmxDefaultDirectory(acs_trace);
	}
	delete (acs_trace);
}

void FIXS_CCH_DiskHandler::createIplbDefaultDirectory(ACS_TRA_trace* _trace)
{

	// create Folder IPLB
	if (!CCH_Util::PathFileExists(m_iplbFolder.c_str()))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " iplbFolder doesn't exists" << std::endl;

		//Upgrade path: Check if /data/IPLB/ path exists
		std::string oldIplbFolder = "/data/IPLB/";
		std::string oldIplbSymLink = "/data/apz/data/IPLB";

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " check if exists old IPLB path: " << oldIplbFolder
				<< std::endl;

		if (CCH_Util::PathFileExists(oldIplbFolder.c_str()))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< " old IPLB path exists. Upgrade it to new path: "
					<< m_iplbFolder << std::endl;

			//Old IPLB Exist: /data/IPLB/
			//move to new path: /data/ext/IPLB/
			std::string iplbFolderMinusData = "/data/ext/IPLB/";

			if (!CCH_Util::CopyFolder(oldIplbFolder.c_str(),
					iplbFolderMinusData.c_str()))
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
						<< " CopyFolder failed from: " << oldIplbFolder
						<< " to: " << iplbFolderMinusData << std::endl;
			}
			else
			{
				//Checks if old IPLB symbolic link exist
				if (CCH_Util::isSymbolicLinkExists(oldIplbSymLink.c_str()))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
							<< " Delete old IPLB symbolic link: "
							<< oldIplbSymLink << std::endl;
					//delete old symlink: /data/apz/data/IPLB
					if (!CCH_Util::deleteFile(oldIplbSymLink.c_str()))
					{
						std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
								<< " Remove Symbolic Link failed: "
								<< oldIplbSymLink << std::endl;
					}
				}

				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
						<< " Remove old IPLB dir: " << oldIplbFolder
						<< std::endl;
				if (!CCH_Util::removeFolder(oldIplbFolder.c_str()))
				{
					std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
							<< " RemoveFolder failed: " << oldIplbFolder
							<< std::endl;
				}
			}
		}
		else
		{
			//Directory doesn't exist, create it...
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< " creating iplbFolder = " << m_iplbFolder.c_str()
					<< std::endl;

			int retValue = ACS_APGCC::create_directories(
					m_iplbFolder.c_str(), ACCESSPERMS);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< " CREATE " << std::endl;
			if (retValue == -1)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
						<< " error = " << std::endl;

				char tmpStr[512] = { 0 };
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"FIXS_CCH - can't create IPLB folder: %s",
						m_iplbFolder.c_str());
				FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON())
					_trace->ACS_TRA_event(1, tmpStr);
			}
			else
			{
				char tmpStr[512] = { 0 };
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"FIXS_CCH - create IPLB folder: %s",
						m_iplbFolder.c_str());
				FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())
					_trace->ACS_TRA_event(1, tmpStr);
			}
		}
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " IPLB folder already exists" << std::endl;

		char tmpStr[512] = { 0 };
		snprintf(tmpStr, sizeof(tmpStr) - 1,
				"FIXS_CCH -  IPLB folder already exist: %s",
				m_iplbFolder.c_str());
		FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())
			_trace->ACS_TRA_event(1, tmpStr);
	}

	//createSymbolicLink for IPLB
	int result = 0;
	std::string link_des = "/data/ext/IPLB";
	std::string link = m_tftpRootFolder + "/IPLB";

	if (!CCH_Util::isSymbolicLinkExists(link.c_str()))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " Creaye IPLB symbolic link: " << link << "-> " << link_des
				<< std::endl;

		result = symlink(link_des.c_str(), link.c_str());

		if (result)
		{
			std::cout << "DBG: " << result << __FUNCTION__ << "@" << __LINE__
					<< " error in createSymbolicLink " << std::endl;
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< " createSymbolicLink is OK!!" << std::endl;
		}
	}
	else
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " IPLB synmolic link aready exists" << std::endl;
	}
}

void FIXS_CCH_DiskHandler::createScxDefaultDirectory(ACS_TRA_trace* _trace)
{
	// create Folder SCX
	if (!CCH_Util::PathFileExists(m_scxFolder.c_str()))
	{
		//Directory doesn't exist, create it...
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " creating m_scxFolder = " << m_scxFolder.c_str()
				<< std::endl;

		int retValue = ACS_APGCC::create_directories(m_scxFolder.c_str(),
				ACCESSPERMS);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CREATE "
				<< std::endl;
		if (retValue == -1)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< " error = " << std::endl;

			{
				char tmpStr[512] = { 0 };
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"FIXS_CCH - can't create SCX folder: %s",
						m_scxFolder.c_str());
				FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON())
					_trace->ACS_TRA_event(1, tmpStr);
			}
		}
		else
		{

			{
				char tmpStr[512] = { 0 };
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"FIXS_CCH - create SCX folder: %s", m_scxFolder.c_str());
				FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())
					_trace->ACS_TRA_event(1, tmpStr);
			}
		}
	}
	else
	{

		{
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"FIXS_CCH - SCX folder already exist: %s",
					m_scxFolder.c_str());
			FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1, tmpStr);
		}
	}
}

void FIXS_CCH_DiskHandler::createEpblDefaultDirectory(ACS_TRA_trace* _trace)
{
	if (!CCH_Util::PathFileExists(m_epb1Folder.c_str()))
	{
		//Directory doesn't exist, create it...
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " creating m_epb1Folder = " << m_epb1Folder.c_str()
				<< std::endl;

		int retValue = ACS_APGCC::create_directories(m_epb1Folder.c_str(),
				ACCESSPERMS);
		if (retValue == -1)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< " error = " << std::endl;
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"FIXS_CCH - can't create EPB1 folder: %s",
					m_epb1Folder.c_str());
			FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_ERROR);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1, tmpStr);
		}
		else
		{
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"FIXS_CCH - create EPB1 folder: %s", m_epb1Folder.c_str());
			FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1, tmpStr);
		}
	}
	else
	{
		char tmpStr[512] = { 0 };
		snprintf(tmpStr, sizeof(tmpStr) - 1,
				"FIXS_CCH - EPB1 folder already exist: %s",
				m_epb1Folder.c_str());
		FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())
			_trace->ACS_TRA_event(1, tmpStr);
	}
}

void FIXS_CCH_DiskHandler::createIptDefaultDirectory(ACS_TRA_trace* _trace)
{
	if (!CCH_Util::PathFileExists(m_iptFolder.c_str()))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " creating m_iptFolder = " << m_iptFolder.c_str()
				<< std::endl;

		int retValue = ACS_APGCC::create_directories(m_iptFolder.c_str(),
				ACCESSPERMS);
		if (retValue == -1)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< " error = " << std::endl;
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"FIXS_CCH - can't create IPT folder: %s",
					m_iptFolder.c_str());
			FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1, tmpStr);
		}
		else
		{
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"FIXS_CCH - create IPT folder: %s", m_iptFolder.c_str());
			FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1, tmpStr);
		}
	}
	else
	{
		char tmpStr[512] = { 0 };
		snprintf(tmpStr, sizeof(tmpStr) - 1,
				"FIXS_CCH - IPT folder already exist: %s", m_iptFolder.c_str());
		FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())
			_trace->ACS_TRA_event(1, tmpStr);

	}
	//std::string iptLog = m_tftpRootFolder + "/IPT/LOGS/";
	/*		std::string iptLog = "/data/IPT/LOGS/";
	 {
	 // create Folder IPT LOGS
	 if (!CCH_Util::PathFileExists(iptLog.c_str()))
	 {
	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " creating iptLog = " << iptLog.c_str() << std::endl;

	 int retValue = ACS_APGCC::create_directories(iptLog.c_str(),ACCESSPERMS);
	 if (retValue == -1)
	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " error = " << std::endl;
	 }
	 }*/
	//createSymbolicLink for IPT
	int result = 0;
	std::string link = "/data/IPT";
	//TODO: Using the Default root folder (/data/apz/data)
	std::string dir = m_tftpRootFolderDefault + "/IPT";
	//result = CCH_Util::createSymbolicLink(link, dir); //An alternative way - system ("junction -s c:\\link.lnk c:\\temp_install");
	result = symlink(link.c_str(), dir.c_str());
	if (result)
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " error in createSymbolicLink " << std::endl;
	else
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " createSymbolicLink is OK!!" << std::endl;
}

void FIXS_CCH_DiskHandler::createCmxDefaultDirectory(ACS_TRA_trace* _trace)
{
	// create Folder CMX
	if (!CCH_Util::PathFileExists(m_cmxFolder.c_str()))
	{
		//Directory doesn't exist, create it...
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " creating m_cmxFolder = " << m_cmxFolder.c_str()
				<< std::endl;

		int retValue = ACS_APGCC::create_directories(m_cmxFolder.c_str(),
				ACCESSPERMS);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CREATE "
				<< std::endl;
		if (retValue == -1)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< " error = " << std::endl;

			{
				char tmpStr[512] = { 0 };
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"FIXS_CCH - can't create CMX folder: %s",
						m_cmxFolder.c_str());
				FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON())
					_trace->ACS_TRA_event(1, tmpStr);
			}
		}
		else
		{

			{
				char tmpStr[512] = { 0 };
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"FIXS_CCH - create CMX folder: %s", m_cmxFolder.c_str());
				FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())
					_trace->ACS_TRA_event(1, tmpStr);
			}
		}
	}
	else
	{

		{
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"FIXS_CCH - CMX folder already exist: %s",
					m_cmxFolder.c_str());
			FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1, tmpStr);
		}
	}
}
void FIXS_CCH_DiskHandler::createIpmiupgDefaultDirectory(ACS_TRA_trace* _trace)
{
        // create Folder CMX
        if (!CCH_Util::PathFileExists(m_ipmiupgFolder.c_str()))
        {
                //Directory doesn't exist, create it...
                std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
                                << " creating m_ipmiupgFolder = " << m_ipmiupgFolder.c_str()
                                << std::endl;

                int retValue = ACS_APGCC::create_directories(m_ipmiupgFolder.c_str(),
                                ACCESSPERMS);
                std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CREATE "
                                << std::endl;
                if (retValue == -1)
                {
                        std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
                                        << " error = " << std::endl;

                        {
                                char tmpStr[512] = { 0 };
                                snprintf(tmpStr, sizeof(tmpStr) - 1,
                                                "FIXS_CCH - can't create ipmiupg folder: %s",
                                                m_ipmiupgFolder.c_str());
                                FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_ERROR);
                                if (_trace->ACS_TRA_ON())
                                        _trace->ACS_TRA_event(1, tmpStr);
                        }
                }
                else
                {
                       {
                                char tmpStr[512] = { 0 };
                                snprintf(tmpStr, sizeof(tmpStr) - 1,
                                                "FIXS_CCH - create ipmiupg folder: %s", m_ipmiupgFolder.c_str());
                                FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
                                if (_trace->ACS_TRA_ON())
                                        _trace->ACS_TRA_event(1, tmpStr);
                        }
                }
        }
        else
        {

                {
                        char tmpStr[512] = { 0 };
                        snprintf(tmpStr, sizeof(tmpStr) - 1,
                                        "FIXS_CCH - ipmiupg folder already exist: %s",
                                        m_ipmiupgFolder.c_str());
                        FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
                        if (_trace->ACS_TRA_ON())
                                _trace->ACS_TRA_event(1, tmpStr);
                }
        }
}

void FIXS_CCH_DiskHandler::createEvoEtDefaultDirectory(ACS_TRA_trace* _trace)
{
	if (!CCH_Util::PathFileExists(m_evoEtFolder.c_str()))
	{
		//Directory doesn't exist, create it...
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " creating m_evoEtFolder = " << m_evoEtFolder.c_str()
				<< std::endl;

		int retValue = ACS_APGCC::create_directories(m_evoEtFolder.c_str(),
				ACCESSPERMS);
		if (retValue == -1)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< " error = " << std::endl;
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"FIXS_CCH - can't create EVOET folder: %s",
					m_evoEtFolder.c_str());
			FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_ERROR);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1, tmpStr);
		}
		else
		{
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"FIXS_CCH - create EVOET folder: %s", m_evoEtFolder.c_str());
			FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1, tmpStr);
		}
	}
	else
	{
		char tmpStr[512] = { 0 };
		snprintf(tmpStr, sizeof(tmpStr) - 1,
				"FIXS_CCH - EVOET folder already exist: %s",
				m_evoEtFolder.c_str());
		FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
		if (_trace->ACS_TRA_ON())
			_trace->ACS_TRA_event(1, tmpStr);
	}
}

void FIXS_CCH_DiskHandler::removeSoftwarePackage(const char *softwarePackage)
{
	if (CCH_Util::removeFolder(softwarePackage) != 0)
	{
		std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__
				<< "Failed to remove folder:" << softwarePackage << std::endl;
	}
}

std::string FIXS_CCH_DiskHandler::getScxFolder()
{
	return m_scxFolder;
}

std::string FIXS_CCH_DiskHandler::getIptFolder()
{
	return m_iptFolder;
}

std::string FIXS_CCH_DiskHandler::getIplbFolder()
{
	return m_iplbFolder;
}

std::string FIXS_CCH_DiskHandler::getTftpRootFolder()
{
	return m_tftpRootFolder;
}

bool FIXS_CCH_DiskHandler::copyIptSoftware(const char* srcFolder,
		const char* dstFolder)
{
	DIR *dp;
	struct dirent *dirp;
	struct stat statbuf;
	char *new_fullpath;
	char *new_filename;

	if (stat(srcFolder, &statbuf) != 0)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " Direcoty: " << srcFolder << " doesn't exist !!! "
				<< std::endl;
		return false;
	}

	if (!S_ISDIR(statbuf.st_mode))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " Is not a Direcoty: " << srcFolder << std::endl;
		return false;
	}

	if ((dp = opendir(srcFolder)) == NULL)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< "Read permission is denied for the directory named: "
				<< srcFolder << std::endl;
		return false;
	}

	//opendir OK
	bool res;

	while ((dirp = readdir(dp)) != NULL)
	{

		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue; //ignore dot and dot-dot

		new_fullpath = (char*) calloc((strlen(srcFolder) + strlen(dirp->d_name)
				+ 2), sizeof(char));
		new_filename = (char*) calloc((strlen(dstFolder) + strlen(dirp->d_name)
				+ 2), sizeof(char));

		strcat(new_fullpath, srcFolder);
		strcat(new_fullpath, "/");
		strcat(new_fullpath, dirp->d_name);

		strcat(new_filename, dstFolder);
		strcat(new_filename, "/");
		strcat(new_filename, dirp->d_name);

		if (stat(new_fullpath, &statbuf) != 0)
		{
			free(new_fullpath);
			free(new_filename);
			new_fullpath = NULL;
			new_filename = NULL;
			continue;
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< " New full path: " << new_fullpath << std::endl;

		if (S_ISDIR(statbuf.st_mode))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< "the current file is a directory it needs copy the entire content "
					<< std::endl;
			//the current file is a directory
			//it needs remove the entire content
			res = ACS_APGCC::create_directories(new_filename, ACCESSPERMS);
			if (!res)
			{
				std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__
						<< " Errorno11:" << errno << std::endl;
				free(new_fullpath);
				free(new_filename);
				closedir(dp);
				return false;
			}

			res = copyIptSoftware(new_fullpath, new_filename);
			if (!res)
			{
				std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__
						<< " Errorno4:" << errno << std::endl;
				free(new_fullpath);
				free(new_filename);
				closedir(dp);
				return false;
			}
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
					<< "is a file: " << new_filename << std::endl;
			res = CCH_Util::CopyFile(new_fullpath, new_filename, false, 4096);
			if (!res)
			{
				std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__
						<< " Errorno5:" << errno << std::endl;
				free(new_fullpath);
				free(new_filename);
				closedir(dp);
				return false;
			}
		}

		free(new_fullpath);
		free(new_filename);

	}// end-while

	//close directory
	if (closedir(dp) < 0)
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< "error to close directory " << std::endl;
		return false;
	}
	else
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__
				<< "close directory " << std::endl;

	return true;
}

std::string FIXS_CCH_DiskHandler::getEpb1Folder()
{
	return m_epb1Folder;
}

std::string FIXS_CCH_DiskHandler::getConfigurationFolder()
{
	std::string path = "/data/apz/data/cch";
	return path;
}

std::string FIXS_CCH_DiskHandler::getTempConfigurationFile()
{
	std::string file = "/data/apz/data/cch/statemachines.tmp";
	return file;
}

std::string FIXS_CCH_DiskHandler::getConfigurationFile()
{
	std::string file = "/data/apz/data/cch/statemachines.cfg";
	return file;
}

std::string FIXS_CCH_DiskHandler::getNbiSwPath(
		CCH_Util::fileMFolderType folderType)
{
	std::string completeFolderPath("");
	char * p_finaloutputdir = new char[100];
	int len = 100;
	bool flag = false;
	ACS_APGCC_CommonLib apgccCommon;
	ACS_APGCC_DNFPath_ReturnTypeT retcode;
	const char * p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE;

	std::cout << "folderType : " << folderType << std::endl;

	switch (folderType)
	{

	case CCH_Util::swPackage:

		p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE;
		break;

	case CCH_Util::swPackageIplb:
		p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE;
		flag = true;
		break;

	case CCH_Util::swPackageScxb:

		p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE_SCXB;
		break;

	case CCH_Util::swPackageEpb1:

		p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE_EPB1;
		break;

	case CCH_Util::swPackageIptb:

		p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE_IPTB;
		break;

	case CCH_Util::swPackageCmxb:

		p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE_CMXB;
		break;

	case CCH_Util::swPackageEvoEt:

		p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE_EVOET;
		break;

	case CCH_Util::swPackageCp:

		p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE_CP;
		break;

	case CCH_Util::swPackageFw:

		p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE_FW;
		break;
	case CCH_Util::swPackageSmxb:

		p_FileMFuncName = IMM_Util::ATT_FILEM_SWPACKAGE_SMXB;
		break;
	default:
		break;

	}

	retcode = apgccCommon.GetFileMPath(p_FileMFuncName, p_finaloutputdir, len);
	if (retcode != ACS_APGCC_DNFPATH_SUCCESS)
	{
		std::cout << "Error in getFileMPath : " << retcode << std::endl;
	}

	std::cout << "p_finaloutputdir: " << p_finaloutputdir << std::endl;

	if (flag)
	{
		completeFolderPath = p_finaloutputdir;
		completeFolderPath = completeFolderPath + "/IPLB/";

		flag = false;
	}
	else
	{
		completeFolderPath = p_finaloutputdir;
	}

	delete[] p_finaloutputdir;

	return completeFolderPath;
}

CCH_Util::EnvironmentType FIXS_CCH_DiskHandler::getEnvironment()
{

	return m_environment;

}

void FIXS_CCH_DiskHandler::setEnvironment(CCH_Util::EnvironmentType env)
{
	m_environment = env;
}

ACS_TRA_Logging* FIXS_CCH_DiskHandler::getLogInstance()
{
	if (FIXS_CCH_logging == NULL)
	{
		FIXS_CCH_logging = new ACS_TRA_Logging();
		FIXS_CCH_logging->Open("CCH");
		FIXS_CCH_logging->Write(" FIXS_CCH - Create Instance for logging !!! ",
				LOG_LEVEL_DEBUG);
	}

	return FIXS_CCH_logging;
}

void FIXS_CCH_DiskHandler::closeLogInstance()
{
	if (FIXS_CCH_logging)
	{
		FIXS_CCH_logging->Write(
				" FIXS_CCH - Destroy Instance for logging !!! ",
				LOG_LEVEL_DEBUG);
		FIXS_CCH_logging->Close();
		delete (FIXS_CCH_logging);
		FIXS_CCH_logging = NULL;
	}
}

std::string FIXS_CCH_DiskHandler::getTftpRootFolderDefault()
{
	return m_tftpRootFolderDefault;
}

void FIXS_CCH_DiskHandler::checkStartAfterRestore()
{
	startAfterRestore = false;
	std::vector < std::string > listFolder;

	std::string backupArea = CCH_Util::BRF_PATH;
	std::string clearFileName = CCH_Util::BRF_CLEAR_FILENAME;
	std::string clearPath = backupArea + clearFileName;
	std::string clearPathValue("");
	std::string cchFileName = CCH_Util::BRF_CCH_FILENAME;

	if (CCH_Util::findFile(clearFileName, backupArea))
	{
		CCH_Util::readFile(clearPath, clearPathValue);

		ACS_APGCC::trimEnd(clearPathValue);

		if (!CCH_Util::findFile(cchFileName, clearPathValue))
		{
			std::string completeCchFilePath = clearPathValue + "/"
					+ cchFileName;
			CCH_Util::createFile(completeCchFilePath);
			startAfterRestore = true;
		}
	}

}

bool FIXS_CCH_DiskHandler::isStartedAfterRestore()
{
	return startAfterRestore;
}

std::string FIXS_CCH_DiskHandler::getCmxFolder()
{

	return m_cmxFolder;

}

std::string FIXS_CCH_DiskHandler::getEvoEtFolder()
{

	return m_evoEtFolder;

}
std::string FIXS_CCH_DiskHandler::getSmxFolder()
{
	return m_smxFolder;
}
void FIXS_CCH_DiskHandler::createSmxDefaultDirectory(ACS_TRA_trace* _trace)
{
	// create Folder SMX
	if (!CCH_Util::PathFileExists(m_smxFolder.c_str()))
	{
		//Directory doesn't exist, create it...
		DEBUG( "DBG: " << __FUNCTION__ << "@" << __LINE__);
		DEBUG(" creating m_smxFolder = " << m_smxFolder.c_str());
		

		int retValue = ACS_APGCC::create_directories(m_smxFolder.c_str(),
				ACCESSPERMS);
		DEBUG( "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CREATE ");
				
		if (retValue == ERROR)
		{
			DEBUG("DBG: " << __FUNCTION__ << "@" << __LINE__<< " error = " );
			{
				char tmpStr[512] = { 0 };
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"FIXS_CCH - can't create SMX folder: %s",
						m_smxFolder.c_str());
				FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON())
					_trace->ACS_TRA_event(1, tmpStr);
			}
		}
		else
		{
			{
				char tmpStr[512] = { 0 };
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"FIXS_CCH - create SMX folder: %s", m_smxFolder.c_str());
				FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())
					_trace->ACS_TRA_event(1, tmpStr);
			}
		}
	}
	else
	{
		{
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,
					"FIXS_CCH - SMX folder already exist: %s",
					m_smxFolder.c_str());
			FIXS_CCH_logging->Write(tmpStr, LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())
				_trace->ACS_TRA_event(1, tmpStr);
		}
	}
}


void FIXS_CCH_DiskHandler::createMisallingnedSwObjects(std::vector<std::string> & p_dnList,std::vector<std::string> & immCxpObject)
{
	int result=0;
	std::vector < std::string > listScx;
	std::vector < std::string > listSmx;
	std::vector < std::string > listEpb1;
	std::vector < std::string > listIptb;
	std::vector < std::string > listCmx;
	std::vector < std::string > listEvoEt;
	std::vector < std::string > listIplb;
	std::cout << " immCxpObject size () = " << immCxpObject.size();
//	std::cout << " immCxpObject1 = : " << immCxpObject[0].c_str() << std::endl;

	if (CCH_Util::getFolderList(listScx, m_scxFolder))
	{
		for (unsigned int i = 0; i < listScx.size(); i++)
		{
			int iPos = listScx[i].find_last_of("/");
			std::string container("");
			if (iPos >= 0)
			{
				// Filename is part of input path. Extract it.
				container = listScx[i].substr(iPos + 1);
			}
			cout << " DBG: Container: " << container.c_str() << endl;

			if(find(immCxpObject.begin(), immCxpObject.end(), container) != immCxpObject.end())
			{
				std::cout << " container Object exist in IMM" << std::endl;
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("container Object exist in IMM ",LOG_LEVEL_WARN);
			}
			else
			{
				std::string file = m_scxFolder + container;
				std::string dn="";
				result=FIXS_CCH_SoftwareMap::getInstance()->CreateBladeSwImmObjects(dn,file,IMM_Util::SCXB);
				if(result!=0)
				{
				  if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("CreateBladeSwImmObjects function failed ",LOG_LEVEL_WARN);
				}
				else {
					p_dnList.push_back(dn);
					 
				}
			}

		}
	}

	if (CCH_Util::getFolderList(listSmx, m_smxFolder))
	{
		for (unsigned int i = 0; i < listSmx.size(); i++)
		{
			int iPos = listSmx[i].find_last_of("/");
			std::string container("");
			if (iPos >= 0)
			{
				// Filename is part of input path. Extract it.
				container = listSmx[i].substr(iPos + 1);
			}

			cout << " DBG: Container: " << container.c_str() << endl;

			if(std::find(immCxpObject.begin(), immCxpObject.end(), container) != immCxpObject.end())
                        {
                                std::cout << " container Object exist in IMM" << std::endl;

                        }
                        else
                        {
                                std::string file = m_smxFolder + container;
				std::string dn="";
				result=FIXS_CCH_SoftwareMap::getInstance()->CreateBladeSwImmObjects(dn,file,IMM_Util::SMXB);
                                if(result!=0){
	                                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("CreateBladeSwImmObjects function failed ",LOG_LEVEL_WARN);
                                }
				else {
					p_dnList.push_back(dn);
				}

                        }

		}
	}

	if (CCH_Util::getFolderList(listEpb1, m_epb1Folder))
	{
		for (unsigned int i = 0; i < listEpb1.size(); i++)
		{
			int iPos = listEpb1[i].find_last_of("/");
			std::string container("");
			if (iPos >= 0)
			{
				// Filename is part of input path. Extract it.
				container = listEpb1[i].substr(iPos + 1);
			}

			cout << " DBG: Container: " << container.c_str() << endl;
			
			if(std::find(immCxpObject.begin(), immCxpObject.end(), container) != immCxpObject.end())
                        {
                                std::cout << " container Object exist in IMM" << std::endl;

                        }
                        else
                        {
                                std::string file = m_epb1Folder + container[i];
				std::string dn="";
				result=FIXS_CCH_SoftwareMap::getInstance()->CreateBladeSwImmObjects(dn,file,IMM_Util::EPB1);
                                if(result!=0){
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("CreateBladeSwImmObjects function failed ",LOG_LEVEL_WARN);
                                }
				else
					p_dnList.push_back(dn);
                        }
		}
	}

	if (CCH_Util::getFolderList(listIptb, m_iptFolder))
	{
		for (unsigned int i = 0; i < listIptb.size(); i++)
		{
			int iPos = listIptb[i].find_last_of("/");
			std::string container("");
			if (iPos >= 0)
			{
				// Filename is part of input path. Extract it.
				container = listIptb[i].substr(iPos + 1);
			}

			cout << " DBG: Container: " << container.c_str() << endl;

			if(std::find(immCxpObject.begin(), immCxpObject.end(), container) != immCxpObject.end())
                        {
                                std::cout << " container Object exist in IMM" << std::endl;

                        }
                        else
                        {
                                std::string file = m_iptFolder + container;
				std::string dn="";
				result=FIXS_CCH_SoftwareMap::getInstance()->CreateBladeSwImmObjects(dn,file,IMM_Util::IPTB);
                                if(result!=0){
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("CreateBladeSwImmObjects function failed ",LOG_LEVEL_WARN);
                                }
				else
					p_dnList.push_back(dn);
                        }
		}
	}

	if (CCH_Util::getFolderList(listCmx, m_cmxFolder))
	{
		for (unsigned int i = 0; i < listCmx.size(); i++)
		{
			int iPos = listCmx[i].find_last_of("/");
			std::string container("");
			if (iPos >= 0)
			{
				// Filename is part of input path. Extract it.
				container = listCmx[i].substr(iPos + 1);
			}

			cout << " DBG: Container: " << container.c_str() << endl;
			if(std::find(immCxpObject.begin(), immCxpObject.end(), container) != immCxpObject.end())
                        {
                                std::cout << " container Object exist in IMM" << std::endl;

                        }
                        else
                        {
                                std::string file = m_cmxFolder + container;
				std::string dn="";
				result=FIXS_CCH_SoftwareMap::getInstance()->CreateBladeSwImmObjects(dn,file,IMM_Util::CMXB);
                                if(result!=0){
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("CreateBladeSwImmObjects function failed ",LOG_LEVEL_WARN);
                                }
				else
					p_dnList.push_back(dn);

                        }
		}
	}

	if (CCH_Util::getFolderList(listIplb, m_iplbFolder))
	{

		for (unsigned int i = 0; i < listIplb.size(); i++)
		{
			int iPos = listIplb[i].find_last_of("/");
			std::string container("");
			if (iPos >= 0)
			{
				// Filename is part of input path. Extract it.
				container = listIplb[i].substr(iPos + 1);
			}

			cout << " DBG: Container: " << container.c_str() << endl;

			if(std::find(immCxpObject.begin(), immCxpObject.end(), container) != immCxpObject.end())
                        {
                                std::cout << " container Object exist in IMM" << std::endl;

                        }
                        else
                        {
                                std::string file = m_iplbFolder + container;
				std::string dn="";
				result=FIXS_CCH_SoftwareMap::getInstance()->CreateBladeSwImmObjects(dn,file,IMM_Util::IPLB);
                                if(result!=0){
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("CreateBladeSwImmObjects function failed ",LOG_LEVEL_WARN);
                                }
				else
					p_dnList.push_back(dn);
                        }
		}

	}

	if (CCH_Util::getFolderList(listEvoEt, m_evoEtFolder))
	{
		for (unsigned int i = 0; i < listEvoEt.size(); i++)
		{
			int iPos = listEvoEt[i].find_last_of("/");
			std::string container("");
			if (iPos >= 0)
			{
				// Filename is part of input path. Extract it.
				container = listEvoEt[i].substr(iPos + 1);
			}

			cout << " DBG: Container: " << container.c_str() << endl;

			if(std::find(immCxpObject.begin(), immCxpObject.end(), container) != immCxpObject.end())
                        {
                                std::cout << " container Object exist in IMM" << std::endl;

                        }
                        else
                        {
                                std::string file = m_evoEtFolder + container;
				std::string dn="";
				result=FIXS_CCH_SoftwareMap::getInstance()->CreateBladeSwImmObjects(dn,file,IMM_Util::EVOET);
                                if(result!=0){
                                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("CreateBladeSwImmObjects function failed ",LOG_LEVEL_WARN);
                                }
				else
					p_dnList.push_back(dn);
                        }
		}
	}
}
