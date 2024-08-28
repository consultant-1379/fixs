// FIXS_CCH_Installer
#include "FIXS_CCH_Installer.h"

#define XMLERROR -1

// Class FIXS_CCH_Installer::SaxHandler

FIXS_CCH_Installer::SaxHandler::SaxHandler (const std::string &xmlFilePath, propertiesMap_t &xmlProperties, FIXS_CCH_Installer::InstallationType& installationType)
      : m_xmlProperties(xmlProperties),
        m_installationType(installationType)
{
	_traceSax = new ACS_TRA_trace("CCH SaxHandler");
	UNUSED(xmlFilePath);
}

FIXS_CCH_Installer::SaxHandler::~SaxHandler()
{
	delete (_traceSax);

}


 void FIXS_CCH_Installer::SaxHandler::startElement (const XMLCh* const  name, AttributeList &attributes)
{

	char* elem = XMLString::transcode(name);
	std::string element(elem);

	if (element == "product")
	{
		std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ <<std::endl;
		for (XMLSize_t i = 0; i < attributes.getLength(); i++)
		{
			char* nm = XMLString::transcode(attributes.getName(i));
			std::string name(nm);
			char* vl = XMLString::transcode(attributes.getValue(i));
			std::string value(vl);

			if (name == "name")
			{
				if (value == "scxb-kernel")
				{
					m_installationType = SCX_INSTALLATION;
					//std::pair<propertiesMap_t::iterator,bool> ret =
					m_xmlProperties.insert(propertiesMap_t::value_type(PRODUCT_NAME,value));
				}
				else if (value == "smxb-kernel")
				{
					m_installationType = SMX_INSTALLATION;
					//std::pair<propertiesMap_t::iterator,bool> ret =
					m_xmlProperties.insert(propertiesMap_t::value_type(PRODUCT_NAME,value));
				}
				else
				{
					XMLString::release(&vl);
					XMLString::release(&nm);
					break;
				}
			}
			else if (name == "id")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(PRODUCT_ID,value));
			}
			else if (name == "rstate")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(PRODUCT_RSTATE, value));
			}
			else
			{
				std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ <<std::endl;
				if (_traceSax->ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "xml error: problem in PRODUCT element");
					_traceSax->ACS_TRA_event(1, tmpStr);

				}

			}

			XMLString::release(&vl);
			XMLString::release(&nm);
		}
	}
	else if (element == "file")
	{
		std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ <<std::endl;
		for (XMLSize_t i = 0; i < attributes.getLength(); i++)
		{
			char* nm = XMLString::transcode(attributes.getName(i));
			std::string name(nm);
			char* vl = XMLString::transcode(attributes.getValue(i));
			std::string value(vl);

			if (name == "relpath")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(FILE_RELPATH, value));
			}
			else if (name == "name")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(FILE_NAME, value));
			}
			else if (name == "provider_prodnr")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(FILE_PROVIDER_PRODNR, value));
			}
			else if (name == "provider_rstate")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(FILE_PROVIDER_RSTATE, value));
			}
			else
			{
			std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ <<std::endl;
				if (_traceSax->ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "xml error: problem in FILE element");
					_traceSax->ACS_TRA_event(1, tmpStr);
				}

			}

			XMLString::release(&vl);
			XMLString::release(&nm);
		}
	}
	//Load Module tags
	else if (element == "version")
	{
		std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ <<std::endl;
		for (XMLSize_t i = 0; i < attributes.getLength(); i++)
		{
			char* nm = XMLString::transcode(attributes.getName(i));
			std::string name(nm);
			char* vl = XMLString::transcode(attributes.getValue(i));
			std::string value(vl);

			if (name == "rel")
			{
				//std::pair<propertiesMap_t::iterator,bool> ret =
				m_xmlProperties.insert(propertiesMap_t::value_type(VERSION_REL,value));
				std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ << " rel= " << value.c_str() << std::endl;
			}
			else
			{
				std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ <<std::endl;
				if (_traceSax->ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] xml error: problem in %s element", __FUNCTION__, __LINE__, element.c_str());
					_traceSax->ACS_TRA_event(1, tmpStr);
				}

				XMLString::release(&vl);
				XMLString::release(&nm);
				break;
			}

			XMLString::release(&vl);
			XMLString::release(&nm);
		}
	}
	else if (element == "package")
	{
		std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ <<std::endl;
		for (XMLSize_t i = 0; i < attributes.getLength(); i++)
		{
			char* nm = XMLString::transcode(attributes.getName(i));
			std::string name(nm);
			char* vl = XMLString::transcode(attributes.getValue(i));
			std::string value(vl);

			if (name == "name")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(PACKAGE_NAME, value));
			}
			else if (name == "id")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(PACKAGE_ID, value));
			}
			else if (name == "rstate")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(PACKAGE_RSTATE, value));
			}
			else if (name == "fbn")
			{
				if (value == "IPTB")
				{
					m_installationType = IPT_INSTALLATION;
				}
				else if (value == "EPB1")
				{
					m_installationType = EPB1_INSTALLATION;
				}
				else if (value == "IPLB")
				{
					m_installationType = IPLB_INSTALLATION;
				}
				else if (value == "CMXB")
				{
					m_installationType = CMX_INSTALLATION;
				}
				else if (value == "EVOET")
				{
					m_installationType = EVOET_INSTALLATION;
				}
				else if (value == "SCXB")
				{
					m_installationType = SCX_INSTALLATION;
				}
				else if (value == "SMXB")
				{
					m_installationType = SMX_INSTALLATION;
				}
				m_xmlProperties.insert(propertiesMap_t::value_type(PACKAGE_FBN, value));
			}
			else
			{
				std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ <<std::endl;
				if (_traceSax->ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] xml error: problem in %s element", __FUNCTION__, __LINE__, element.c_str());
					_traceSax->ACS_TRA_event(1, tmpStr);
				}

				XMLString::release(&vl);
				XMLString::release(&nm);
				break;
			}

			XMLString::release(&vl);
			XMLString::release(&nm);
		}
	}
	else if (element == "load_module")
	{
		std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ <<std::endl;
		for (XMLSize_t i = 0; i < attributes.getLength(); i++)
		{
			char* nm = XMLString::transcode(attributes.getName(i));
			std::string name(nm);
			char* vl = XMLString::transcode(attributes.getValue(i));
			std::string value(vl);
			
			if (name == "relpath")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(LM_REL_PATH, value));
			}
			else if (name == "bootfilename")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(LM_BOOTFILE_NAME, value));
			}
			else if (name == "prodnr")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(LM_PRODUCT_NUMBER, value));
			}
			else if (name == "rstate")
			{
				m_xmlProperties.insert(propertiesMap_t::value_type(LM_RSTATE, value));
			}
			else if (name == "swVerType")
			{
				std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ << " swVerType = " << value.c_str() << std::endl;
				m_xmlProperties.insert(propertiesMap_t::value_type(LM_SW_VER_TYPE, value));
			}
			else
			{
				std::cout<< "DBG: " << __FUNCTION__ << " " <<__LINE__ <<std::endl;
				if (_traceSax->ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] xml error: problem in %s element", __FUNCTION__, __LINE__, element.c_str());
					_traceSax->ACS_TRA_event(1, tmpStr);
				}

				XMLString::release(&vl);
				XMLString::release(&nm);
				break;
			}
			XMLString::release(&vl);
			XMLString::release(&nm);
		}
	}

	XMLString::release(&elem);

}

 void FIXS_CCH_Installer::SaxHandler::warning (const SAXParseException &exception)
{
  	//trace
	 UNUSED(exception);
}

 void FIXS_CCH_Installer::SaxHandler::error (const SAXParseException &exception)
{
	//trace
	 UNUSED(exception);
}

 void FIXS_CCH_Installer::SaxHandler::fatalError (const SAXParseException &exception)
{
	//trace
	 UNUSED(exception);
}

FIXS_CCH_Installer::FIXS_CCH_Installer ()
:m_archiveFullPath(""),m_installationType(UNKNOWN), m_swVerType(DEFAULT_SW_VER_TYPE)
{
	m_fileInPackage = "";

	memset(m_xmlFile, 0, sizeof(m_xmlFile));

	_trace = new ACS_TRA_trace("CCH Installer");
	
	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

}
FIXS_CCH_Installer::FIXS_CCH_Installer (std::string archiveFullPath)
:m_archiveFullPath(archiveFullPath), m_installationType(UNKNOWN), m_swVerType(DEFAULT_SW_VER_TYPE)
{
	_trace = new ACS_TRA_trace("CCH Installer");

	m_fileInPackage = "";

	memset(m_xmlFile, 0, sizeof(m_xmlFile));

	std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " - m_archiveFullPath == " << m_archiveFullPath.c_str() << std::endl;

    std::string appString;

	//get path without any file e.g. /data/opt/ap/nbi/sw_package
	m_archivePath = m_archiveFullPath.substr(0,(m_archiveFullPath.find_last_of("/")+1));

	std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " - m_archivePath == " << m_archivePath.c_str() << std::endl;

	//get file name
	m_file = m_archiveFullPath.substr(m_archiveFullPath.find_last_of("/")+1); //19010-CXP9......tar.gz - .tar - .zip

	std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " - m_file == " << m_file.c_str() << std::endl;


	// build "K:\\TEMP_INSTALL\\19010-CXP9......tar.gz" file name
	m_cmdFolder = CCH_Util::TEMP_FOLDER + "/" + m_file;

	std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " - m_cmdFolder == " << m_cmdFolder.c_str() << std::endl;

	if(CCH_Util::CheckExtention(m_cmdFolder.c_str(),".zip"))
		m_extract= "unzip \"" + m_cmdFolder +"\" -d \"" + CCH_Util::TEMP_FOLDER + "\"";


	if(CCH_Util::CheckExtention(m_cmdFolder.c_str(),".tar"))
		m_extract="tar xfv \"" + m_cmdFolder +"\" -C \"" + CCH_Util::TEMP_FOLDER + "\"";

	if(CCH_Util::CheckExtention(m_cmdFolder.c_str(),".tar.gz"))
		m_extract 	= "tar zxfv \"" + m_cmdFolder +"\" -C \"" + CCH_Util::TEMP_FOLDER + "\"";

	std::cout << "DBG:" << __FUNCTION__ << "@" << __LINE__ << " - m_extract == " << m_extract.c_str() << std::endl;

	memset(m_cmdFolderTar, 0, sizeof(m_cmdFolderTar));
	memset(m_cmdFolderZip, 0, sizeof(m_cmdFolderZip));

	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();

}


FIXS_CCH_Installer::~FIXS_CCH_Installer()
{
	clean();
	delete (_trace);
	FIXS_CCH_logging = 0;
}


 int FIXS_CCH_Installer::install (int fbn)
{
    if (m_xmlProperties.empty())
	{
		//error. no properties have been read!!
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. no properties have been read!");
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		}
		return XMLERROR;
	}

    if (m_installationType == FIXS_CCH_Installer::UNKNOWN)
    {
    	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " UNKNOWN"  << std::endl;
    	return XMLERROR;
    }
    else if (m_installationType == FIXS_CCH_Installer::SCX_INSTALLATION)
    {
    	if (fbn == IMM_Util::SCXB){
    		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " INSTALLING SCX"  << std::endl;
    		return installScx();
    	}
    	else return PACKAGE_NOT_APPLICABLE;
    }
    else if (m_installationType == FIXS_CCH_Installer::IPT_INSTALLATION)
    {
    	if (fbn == IMM_Util::IPTB){
    		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " INSTALLING IPT"  << std::endl;
    		return installipt();
    	}
    	else return PACKAGE_NOT_APPLICABLE;
    }
    else if (m_installationType == FIXS_CCH_Installer::EPB1_INSTALLATION)
    {
    	if (fbn == IMM_Util::EPB1){
    		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " INSTALLING EPB1"  << std::endl;

    		return installEpb1();
    	}
    	else return PACKAGE_NOT_APPLICABLE;
    }
    else if (m_installationType == FIXS_CCH_Installer::CMX_INSTALLATION)
    {
    	if (fbn == IMM_Util::CMXB){
    		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " INSTALLING CMX"  << std::endl;

    		return installCmx();
    	}
    	else return PACKAGE_NOT_APPLICABLE;
    }
    else if (m_installationType == FIXS_CCH_Installer::EVOET_INSTALLATION)
    {
    	if (fbn == IMM_Util::EVOET){
    		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " INSTALLING EVOET"  << std::endl;

    		return installEvoEt();
    	}
    	else return PACKAGE_NOT_APPLICABLE;
    }
    else if (m_installationType == FIXS_CCH_Installer::IPLB_INSTALLATION)
    {
    	if (fbn == IMM_Util::IPLB){
    		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " INSTALLING IPLB"  << std::endl;

    		return installIplb();
    	}
    	else return PACKAGE_NOT_APPLICABLE;
    }
    else if (m_installationType == FIXS_CCH_Installer::SMX_INSTALLATION)
    {
    	if (fbn == IMM_Util::SMXB){
    		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " INSTALLING SMX"  << std::endl;

    		return installSmx();
    	}
        else return PACKAGE_NOT_APPLICABLE;
    }
    else
    	return EXECUTED;


}

 void FIXS_CCH_Installer::clean ()
{
	FIXS_CCH_DiskHandler::deleteFile( (m_archivePath + m_file).c_str() );
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_Installer - cleaning environment...",LOG_LEVEL_DEBUG);
	if (!FIXS_CCH_DiskHandler::deleteTempFiles())
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_Installer - failed to delete temp files...",LOG_LEVEL_ERROR);
}

 bool FIXS_CCH_Installer::initialize ()
{

	FIXS_CCH_DiskHandler::deleteTempFiles();

	// CCH_Util::TEMP_FOLDER check
	if(!CCH_Util::PathFileExists(CCH_Util::TEMP_FOLDER.c_str()))
	{
		//CCH_Util::TEMP_FOLDER doesn't exist, create it...
	    ACS_APGCC::create_directories(CCH_Util::TEMP_FOLDER.c_str(),ACCESSPERMS);
	    //createDirectory(CCH_Util::TEMP_FOLDER.c_str(), NULL); //LPSECURITY_ATTRIBUTES attr = NULL;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	}
	else
	{
		//CCH_Util::TEMP_FOLDER already exists
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("TEMP_FOLDER already exists",LOG_LEVEL_ERROR);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	}

	if(!CCH_Util::PathFileExists(m_archivePath.c_str()))
	{
		//error the directory doesn't exist
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] The archive path %s does not exist.", __FUNCTION__, __LINE__, m_archivePath.c_str());
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
		return false;
	}
	else
	{
		//m_archivePath directory found
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CopyFile(from = " << m_archiveFullPath.c_str() << ", to = " << m_cmdFolder.c_str() << ")" << std::endl;
		if(!CCH_Util::PathFileExists(m_archiveFullPath.c_str())){

			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
			return false;
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

		if (CCH_Util::CopyFile(m_archiveFullPath.c_str(), m_cmdFolder.c_str(), false, 4096) == false )
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error during copy!" << std::endl;
			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Cannot copy \"%s\" to \"%s\"", __FUNCTION__, __LINE__, m_archiveFullPath.c_str(), m_cmdFolder.c_str());
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
			return false;
		}


		// Execute unzip commands..
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " - execute : " << m_extract.c_str() << std::endl;
		if ( (system(m_extract.c_str())) == 0)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Extract ok!" << std::endl;
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_Installer - Extract ok !!!",LOG_LEVEL_DEBUG);
		}
		else
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ <<" ERROR IN FILE GZ......."<<std::endl;

			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error during extract file.");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}

			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] unzip command failed: \"%s\"", __FUNCTION__, __LINE__, m_extract.c_str());
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
			return false;
		}

		//get and set xml file name from TEMP_INSTALL folder
		if (!this->setXmlFile())
		{
			//error
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error during settig XML file!!" << std::endl;
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error when setting xml file.");
				_trace->ACS_TRA_event(1, tmpStr);
			}

			char traceChar[512] = {0};
			snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Wrong Installation file.", __FUNCTION__, __LINE__);
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
			return false;
		}

		return initializeParser();

	}//else
}

 bool FIXS_CCH_Installer::initializeParser ()
{
	bool errorOccurred = false;

    try
    {
        XMLPlatformUtils::Initialize();
    }
	catch (const XMLException& e)
    {
       // trace StrX(e.getMessage())
		{ //trace
			char* xmlExp = XMLString::transcode(e.getMessage());
			std::string xmlException(xmlExp);
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "XMLException: %s", xmlException.c_str() );
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			XMLString::release(&xmlExp);
		}
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] XML Exception. Cannot initialize Xerces parser.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
		return false;
	}

	SAXParser* parser = new SAXParser;

    parser->setValidationScheme(SAXParser::Val_Auto);
    parser->setDoNamespaces(false);
    parser->setDoSchema(false);
    parser->setValidationSchemaFullChecking(false);

    //  Create our SAX handler object and install it on the parser
	SaxHandler handler(this->m_xmlFile, this->m_xmlProperties, this->m_installationType);
    parser->setDocumentHandler(&handler);
    parser->setErrorHandler(&handler);

	try
	{
		parser->parse(m_xmlFile);
	}
	catch (const OutOfMemoryException& e)
	{
		//trace "OutOfMemoryException"
		{ //trace
			char tmpStr[512] = {0};
			char* xmlExp = XMLString::transcode(e.getMessage());
			std::string xmlException(xmlExp);
			snprintf(tmpStr, sizeof(tmpStr) - 1, "OutOfMemoryException: %s", xmlException.c_str() );
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			XMLString::release(&xmlExp);
		}
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] OutOfMemoryException. Cannot parse xml file %s.", __FUNCTION__, __LINE__, m_xmlFile);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
		errorOccurred = true;
	}
	catch (const XMLException& e)
	{
		{ //trace
			char tmpStr[512] = {0};
			char* xmlExp = XMLString::transcode(e.getMessage());
			std::string xmlException(xmlExp);
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error during parsing xml file: %s", xmlException.c_str() );
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			XMLString::release(&xmlExp);
		}

		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] XMLException. Error parsing xml file %s. ", __FUNCTION__, __LINE__, m_xmlFile);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
		errorOccurred = true;
	}
	catch (...)
	{
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Unexpected exception during parsing");
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		}

		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Generic Exception got parsing xml file %s.", __FUNCTION__, __LINE__, m_xmlFile);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
		errorOccurred = true;
	}

	delete parser;

    // And call the termination method
    XMLPlatformUtils::Terminate();

    return (!errorOccurred);

}

 bool FIXS_CCH_Installer::setXmlFile ()
{


	bool result = false;

	char SwpDirSpec[1000] = {0};

	snprintf(SwpDirSpec, sizeof(SwpDirSpec) - 1, "%s", CCH_Util::TEMP_FOLDER.c_str());

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Finding .xml file!" << std::endl;

	char type[10]={0};
	int dimErr=0;
	std::vector<std::string> xmlfiles;
	sprintf(type,"%s",".xml");

	dimErr=CCH_Util::findFile(SwpDirSpec,type,xmlfiles);

	if(xmlfiles.size()>0)
	{
		snprintf(m_xmlFile, sizeof(m_xmlFile) - 1, "%s", xmlfiles[0].c_str());
		result = true;

	}
	else
	{

		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Wrong Installation file. Cannot find any xml file at \"%s\"", __FUNCTION__, __LINE__, SwpDirSpec);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
		return false;
	}


	return result;

}

 std::string FIXS_CCH_Installer::getInstalledPackage ()
{

	return m_installedPackage;

}

 int FIXS_CCH_Installer::getCmxVersionType ()
{
	return m_swVerType;	
}

 bool FIXS_CCH_Installer::setTarFile ()
{
  	bool result = false;
	std::vector<std::string> tarfiles;
	char type[10]={0};
	int dimErr = 0;
	char SwpDirSpecTar [MAX_PATH_CCH] = {0};
	char extTar [MAX_PATH_CCH] = {0};

	sprintf(type,"%s",".tar");

	snprintf(SwpDirSpecTar, sizeof(SwpDirSpecTar) - 1, "%s/*.tar", CCH_Util::TEMP_FOLDER.c_str());

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Finding .tar packages!" << std::endl;

	dimErr=CCH_Util::findFile(SwpDirSpecTar,type,tarfiles);

	if(tarfiles.size()>0)
	{
		snprintf(m_cmdFolderTar, sizeof(m_cmdFolderTar) - 1, "%s", tarfiles[0].c_str());
		std::cout << "m_cmdFolderTar: "<< m_cmdFolderTar <<std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "m_cmdFolderTar!" << std::endl;
		result = true;

		snprintf(extTar,sizeof(extTar),"tar xfv \"%s\" -C \"%s\"",m_cmdFolderTar,CCH_Util::TEMP_FOLDER.c_str());

		m_extractTar=extTar;

	}
	else
		result = false;

  	return result;
}

 std::string FIXS_CCH_Installer::getInstalledProduct ()
{
	return m_installedProduct;
}

 std::string FIXS_CCH_Installer::getFileNameInPackage ()
 {
 	return m_fileInPackage;
 }

 void FIXS_CCH_Installer::uninstall ()
{
	if (m_finalSourceFolder.length() != 0)
	{
		//FIXS_CCH_DiskHandler::removeSoftwarePackage(m_finalSourceFolder.c_str());
		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		//delete "CXP9014804_R5J01" folder

		{ //trace
			char tmpStr[512] = { 0 };
			snprintf(tmpStr, sizeof(tmpStr) - 1,"Error During removeSoftwarePackage method!");
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		}

	}
}

 bool FIXS_CCH_Installer::findTarFile ()
{

	bool result=false;
	char SwpDirSpecTar [1000] = {0};
	char extTar [1000] = {0};
	std::vector<std::string> tarfiles;
	int dimErr = 0;

	snprintf(SwpDirSpecTar, sizeof(SwpDirSpecTar) - 1, "%s", CCH_Util::TEMP_FOLDER.c_str());

	std::cout << "SwpDirSpecTar: "<< SwpDirSpecTar <<std::endl;

	char type[10]={0};
	//int arrleng=0;

	sprintf(type,"%s",".tar");

	dimErr = CCH_Util::findFile(SwpDirSpecTar,type,tarfiles);

	if(tarfiles.size()>0)
	{
		snprintf(m_cmdFolderTar, sizeof(m_cmdFolderTar) - 1, "%s", tarfiles[0].c_str());
		std::cout << "m_cmdFolderTar: "<< m_cmdFolderTar <<std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "m_cmdFolderTar!" << std::endl;
		result = true;

		snprintf(extTar,sizeof(extTar),"tar xfv \"%s\" -C \"%s\"",m_cmdFolderTar,CCH_Util::TEMP_FOLDER.c_str());

		m_extractTar=extTar;

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_extractTar = " << m_extractTar << std::endl;

	}
	else
		result = false;
	return result;
}

 bool FIXS_CCH_Installer::findZipFile ()
{
  	bool result = false;

	char SwpDirSpecZip [MAX_PATH_CCH] = {0};
	char extZip [MAX_PATH_CCH] = {0};

	snprintf(SwpDirSpecZip, sizeof(SwpDirSpecZip) - 1, "%s", CCH_Util::TEMP_FOLDER.c_str());

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Finding .zip packages!" << std::endl;

	char type[10]={0};
	int dimErr = 0;
	std::vector<std::string> zipfiles;


	sprintf(type,"%s",".zip");

	dimErr = CCH_Util::findFile(SwpDirSpecZip,type,zipfiles);

	if(zipfiles.size()>0)
	{
		snprintf(m_cmdFolderZip, sizeof(m_cmdFolderZip) - 1, "%s", zipfiles[0].c_str());

		std::cout << "m_cmdFolderZip: "<< m_cmdFolderZip <<std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "m_cmdFolderZip!" << std::endl;

		result = true;

		//m_extractZip ="7za -o\"" + CCH_Util::TEMP_FOLDER + "\" x \"" + m_cmdFolderZip + "\"";

		snprintf(extZip,sizeof(extZip),"unzip \"%s\" -d \"%s\"",m_cmdFolderZip,CCH_Util::TEMP_FOLDER.c_str());

//		m_extractTar="tar xfv \"" + string(m_cmdFolderTar).c_str() +"\" -C \"" + CCH_Util::TEMP_FOLDER + "\"";

		m_extractTar=extZip;
//		m_extractTar="tar xfv \"" + m_cmdFolderTar +"\" -C \"" + CCH_Util::TEMP_FOLDER + "\"";

		//m_extractZip= "unzip \"" + std::string(m_cmdFolderZip).c_str()+"\" -d \"" + CCH_Util::TEMP_FOLDER + "\"";

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_extractZip = " << m_extractZip << std::endl;


	}
	else
		result = false;

	return result;
}

 FIXS_CCH_Installer::InstallationType FIXS_CCH_Installer::getInstallationType ()
{
	return m_installationType;
}

 int FIXS_CCH_Installer::installScx ()
{
	std::string sourceSubFolder("");
	std::string realPath("");
	std::string scxFile("");
	std::string cmdTempFolderScx("");
	std::string cmdDestFolderScx("");
	std::string softwareRstate("");
	std::string softwarePrNumber("");

	bool newVersion = false;

	xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PRODUCT_ID);
	if (it == m_xmlProperties.end())
	{
		xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PACKAGE_ID);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_ID or PRODUCT_ID not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			sourceSubFolder = (*it).second;
			newVersion = true;
		}
	}
	else
	{
		sourceSubFolder = (*it).second;
	}

	//xmlPropertiesMap_t::iterator
	if (!newVersion)
	{
		it = m_xmlProperties.find(PRODUCT_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PRODUCT_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//set source final subfolder using xml-metafile field
			sourceSubFolder = sourceSubFolder + "_" + (*it).second;  //example id="CXP9014804" rstate="R5J01" sourceSubFolder = CXP9014804_R5J01
			m_installedPackage = sourceSubFolder;
		}


		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(FILE_RELPATH);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_REALPATH not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			//Get realpath of the file SCX.tar from the xml-metafile field
			realPath = (*it).second; // example SCXB-LM_CXC138894/scxb_lm-R5J01/priv/img
			//replace(realPath.begin(), realPath.end(), '/', '\\' );
		}
		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(FILE_NAME);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_NAME not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			//Get realpath of the file SCX.tar from the xml-metafile field
			scxFile = (*it).second; // example SCX.tar
		}
		it = m_xmlProperties.find(FILE_PROVIDER_PRODNR);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_PROVIDER_PRODNR not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			m_installedProduct = (*it).second;
			softwarePrNumber = (*it).second;
		}


		it = m_xmlProperties.find(FILE_PROVIDER_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_PROVIDER_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			m_installedProduct = m_installedProduct + "_" + (*it).second;
			softwareRstate = (*it).second;
		}

	}
	else
	{
		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(PACKAGE_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//set source final subfolder using xml-metafile field
			sourceSubFolder = sourceSubFolder + "_" + (*it).second;  //example id="CXP9014804" rstate="R5J01" sourceSubFolder = CXP9014804_R5J01
			m_installedPackage = sourceSubFolder;
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(LM_REL_PATH);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_REL_PATH not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//Get realpath of the file SCX.tar from the xml-metafile field
			realPath = (*it).second; // example SCXB-LM_CXC138894/scxb_lm-R5J01/priv/img
			//replace(realPath.begin(), realPath.end(), '/', '\\' );
		}

		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(LM_BOOTFILE_NAME);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_BOOTFILE_NAME not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//Get realpath of the file EPB1.tar from the xml-metafile field
			scxFile = (*it).second; // example EPB1.tar
		}

		it = m_xmlProperties.find(LM_PRODUCT_NUMBER);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_PRODUCT_NUMBER not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			m_installedProduct = (*it).second;
			softwarePrNumber = (*it).second;
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		it = m_xmlProperties.find(LM_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			m_installedProduct = m_installedProduct + "_" + (*it).second;
			softwareRstate = (*it).second;
		}
	}

	//check if the package is marked as faulty
	if (!FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(m_installedPackage))
	{
		if (FIXS_CCH_SoftwareMap::getInstance()->isContainerInstalled(m_installedPackage))
		{
			//package already installed
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Package %s is already installed.",__FUNCTION__, __LINE__, m_installedPackage.c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}
			return PACKAGE_ALREADY_INSTALLED;
		}
		if (FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(m_installedProduct, CCH_Util::SCXB))
		{
			//product already installed i.e CXC
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Product %s is already installed. Wrong package %s",__FUNCTION__, __LINE__, m_installedProduct.c_str(), m_installedPackage.c_str());
				std::cout << "DBG: " << tmpStr << std::endl;
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}
			return PACKAGE_ALREADY_INSTALLED;
		}
	}

	//Set enviroment variables, create final destination folder and copy file
	// build m_finalSourceFolder directory
	std::string scxBasePath = FIXS_CCH_DiskHandler::getScxFolder();
	m_finalSourceFolder = scxBasePath + sourceSubFolder + "/"; //example K:/IPT/DATA/CXP9014804_R5J01/


    // build "K:/TEMP_INSTALL/SCXB-LM_CXC138894/scxb_lm-R5J01/priv/img/SCX.tar"
	cmdTempFolderScx = CCH_Util::TEMP_FOLDER + "/" + realPath + scxFile;
	// build "K:/APZ/DATA/BOOT/SCX/CXP9014804_R5J01/SCX.tar" file name
	cmdDestFolderScx = m_finalSourceFolder + scxFile;
	//file name extracted from package
	m_fileInPackage = scxFile;
    //verify if the final destination folder exist. If not, we create it.

	if(!CCH_Util::PathFileExists(m_finalSourceFolder.c_str()))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CreateDirectory(" << m_finalSourceFolder.c_str() << ")" << std::endl;
		int retValue = ACS_APGCC::create_directories(m_finalSourceFolder.c_str(),ACCESSPERMS);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " create ok:" << retValue << std::endl;
	}
	else
	{
		std::string traceStr = "Directory ";
		traceStr += m_finalSourceFolder;
		traceStr += ", already present.";
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceStr.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceStr.c_str(),LOG_LEVEL_ERROR);
	}

	if (CCH_Util::CopyFile(cmdTempFolderScx.c_str(), cmdDestFolderScx.c_str(), false, 4096) == false)
	{
		//error
		//Svuoto directory K:/APZ/DATA/BOOT/SCX/CXP9014804_R5J01/ da tutti i file (scx.tar e conf.ini)
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdTempFolderScx: " << cmdTempFolderScx.c_str() << "\n" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderScx: " << cmdDestFolderScx.c_str() << "\n" << std::endl;
		//Store error nr of copyFile
		int errCopy = errno;

		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		//Check for Disk Full
		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;

	}
	
	std::string xmlFilePath(m_xmlFile);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << std::endl;
	int iPos = xmlFilePath.find_last_of("/");
	std::string cmdDestFolderxml = m_finalSourceFolder + xmlFilePath.substr(iPos+1);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << std::endl;

	if (CCH_Util::CopyFile(xmlFilePath.c_str(), cmdDestFolderxml.c_str(),false, 4096) == false)
	{
		//error
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << "\n" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << "\n" << std::endl;
		//Store error nr of copyFile
		int errCopy = errno;

		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		//Check for Disk Full
		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;

	}

	return EXECUTED;
}

 int FIXS_CCH_Installer::installipt ()
{
	std::string sourceSubFolder(""); //used to store PACKAGE_ID + "_" + PACKAGE_RSTATE
	std::string relativePath(""); // read from LM_REL_PATH; it is the relative path where the boot file is placed
	std::string bootFile(""); // IPT.tar
	std::string cmdTempFolderrelativePath("");
	std::string cmdTempFolderLm("");
	std::string cmdDestFolderLm("");
	std::string softwareRstate(""); // LM_RSTATE
	std::string softwarePrNumber(""); // read from LM_PRODUCT_NUMBER, used to store the product number

	// get properties
	xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PACKAGE_ID);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_ID not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	    }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property PACKAGE_ID not set!" << std::endl;
//		FIXS_CCH_Logger::createInstance()->log("Error. xml property PACKAGE_ID not set!", FIXS_CCH_Logger::Info);

		return XMLERROR;
	}
	else
	{
		sourceSubFolder = (*it).second;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	//xmlPropertiesMap_t::iterator
	it = m_xmlProperties.find(PACKAGE_RSTATE);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_RSTATE not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	    }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property PACKAGE_RSTATE not set!" << std::endl;
	//	FIXS_CCH_Logger::createInstance()->log("Error. xml property PACKAGE_RSTATE not set!", FIXS_CCH_Logger::Info);

		return XMLERROR;
	}
	else
	{
		//set source final subfolder using xml-metafile field
		sourceSubFolder = sourceSubFolder + "_" + (*it).second;  //example id="CXP9014804" rstate="R5J01" sourceSubFolder = CXP9014804_R5J01
		m_installedPackage = sourceSubFolder;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	//xmlPropertiesMap_t::iterator
	it = m_xmlProperties.find(LM_REL_PATH);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_REL_PATH not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	    }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property LM_REL_PATH not set!" << std::endl;
	//	FIXS_CCH_Logger::createInstance()->log("Error. xml property LM_REL_PATH not set!", FIXS_CCH_Logger::Info);

		return XMLERROR;
	}
	else
	{
		//Get relativePath of the file SCX.tar from the xml-metafile field
		relativePath = (*it).second; // example SCXB-LM_CXC138894/scxb_lm-R5J01/priv/img
	    //replace(relativePath.begin(), relativePath.end(), '/', '\\' );
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	//xmlPropertiesMap_t::iterator
	it = m_xmlProperties.find(LM_BOOTFILE_NAME);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_BOOTFILE_NAME not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	    }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property LM_BOOTFILE_NAME not set!" << std::endl;
	//	FIXS_CCH_Logger::createInstance()->log("Error. xml property LM_BOOTFILE_NAME not set!", FIXS_CCH_Logger::Info);

		return XMLERROR;
	}
	else
	{
		//Get relativePath of the file IPT.tar from the xml-metafile field
		bootFile = (*it).second; // example IPT.tar
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	it = m_xmlProperties.find(LM_PRODUCT_NUMBER);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_PRODUCT_NUMBER not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	    }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property LM_PRODUCT_NUMBER not set!" << std::endl;
	//	FIXS_CCH_Logger::createInstance()->log("Error. xml property LM_PRODUCT_NUMBER not set!", FIXS_CCH_Logger::Info);

		return XMLERROR;
	}
	else
	{
		 m_installedProduct = (*it).second;
		 softwarePrNumber = (*it).second;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	it = m_xmlProperties.find(LM_RSTATE);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_RSTATE not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	    }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property LM_RSTATE not set!" << std::endl;
	//	FIXS_CCH_Logger::createInstance()->log("Error. xml property LM_RSTATE not set!", FIXS_CCH_Logger::Info);

		return XMLERROR;
	}
	else
	{
		m_installedProduct = m_installedProduct + "_" + (*it).second;
		softwareRstate = (*it).second;
	}

	//check if the package is marked as faulty
	if (!FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(m_installedPackage))
	{

		if (FIXS_CCH_SoftwareMap::getInstance()->isContainerInstalled(m_installedPackage))
		{
			//package already installed

			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Package %s is already installed.",
						__FUNCTION__, __LINE__, m_installedPackage.c_str());
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				//		FIXS_CCH_Logger::createInstance()->log(tmpStr, FIXS_CCH_Logger::Error);
				std::cout << "DBG: " << tmpStr << std::endl;
			}
			return PACKAGE_ALREADY_INSTALLED;
		}

		if (FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(m_installedProduct, CCH_Util::IPTB))
		{
			//product already installed i.e CXC
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"[%s@%d] Product %s is already installed. Wrong package %s",
						__FUNCTION__, __LINE__, m_installedProduct.c_str(), m_installedPackage.c_str());
				std::cout << "DBG: " << tmpStr << std::endl;
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				//		FIXS_CCH_Logger::createInstance()->log(tmpStr, FIXS_CCH_Logger::Error);
			}
			return PACKAGE_ALREADY_INSTALLED;
		}
	}
	//------------------------
	//Set enviroment variables
	//------------------------
	// build m_finalSourceFolder directory
	std::string iptBasePath = FIXS_CCH_DiskHandler::getIptFolder();

	m_finalSourceFolder = iptBasePath + "/" + sourceSubFolder + "/"; //example K:/IPT/DATA/CXP9014804_R5J01/
    cmdTempFolderrelativePath = CCH_Util::TEMP_FOLDER + "/" + relativePath + "/"; // build "C:/TEMP_INSTALL/IPT_LM_138894/"
	cmdTempFolderLm = CCH_Util::TEMP_FOLDER + "/" + relativePath + "/" + bootFile; // build "C:/TEMP_INSTALL/IPT_LM_138894/IPT.tar"
	cmdDestFolderLm = m_finalSourceFolder + bootFile; // build "K:/IPT/DATA/CXP9014804_R5J01/IPT.tar" file name

	//file name extracted from package
	m_fileInPackage = bootFile;

	//debugging
	std::cout << "DBG: m_finalSourceFolder " << m_finalSourceFolder << std::endl;
	std::cout << "DBG: cmdTempFolderrelativePath " << cmdTempFolderrelativePath << std::endl;
	std::cout << "DBG: cmdTempFolderLm " << cmdTempFolderLm << std::endl;
	std::cout << "DBG: cmdDestFolderLm " << cmdDestFolderLm << std::endl;

	if(!CCH_Util::PathFileExists(cmdTempFolderLm.c_str()))
	{
		{ //trace

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] cannot find the boot file: %s in relative path: %s",
				__FUNCTION__, __LINE__, bootFile.c_str(), relativePath.c_str());
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	//		FIXS_CCH_Logger::createInstance()->log(tmpStr, FIXS_CCH_Logger::Error);
	    }
		return XMLERROR;
	}

	//verify if the final destination folder exist. If not, we create it.
	if(!CCH_Util::PathFileExists(m_finalSourceFolder.c_str()))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CreateDirectory(" << m_finalSourceFolder.c_str() << ")" << std::endl;

		int retValue = ACS_APGCC::create_directories(m_finalSourceFolder.c_str(),ACCESSPERMS);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " create ok:" << retValue << std::endl;

	}
	else
	{
		std::string traceStr = "Directory ";
		traceStr += m_finalSourceFolder;
		traceStr += ", already present.";
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceStr.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << traceStr.c_str() << std::endl;
	//	FIXS_CCH_Logger::createInstance()->log(traceStr, FIXS_CCH_Logger::Info);
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_finalSourceFolder = " << m_finalSourceFolder.c_str() << std::endl;

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

/*	char cmdTempFolderrelativePathChar[MAX_PATH] = {0};
	snprintf(cmdTempFolderrelativePathChar, sizeof(cmdTempFolderrelativePathChar) - 1, "%s",cmdTempFolderrelativePath.c_str());
	char  m_finalSourceFolderChar[MAX_PATH] = {0};
	snprintf(m_finalSourceFolderChar, sizeof(m_finalSourceFolderChar) - 1, "%s", m_finalSourceFolder.c_str());
*/

	if (!FIXS_CCH_DiskHandler::copyIptSoftware(cmdTempFolderrelativePath.c_str(),m_finalSourceFolder.c_str()))
	{

		//Check for Disk Full		
		 if (errno == EDQUOT){
			CCH_Util::removeFolder(m_finalSourceFolder.c_str());
			return DISK_FULL;
		}
		//error in copy
		CCH_Util::removeFolder(m_finalSourceFolder.c_str());

		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed. ", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << tmpStr << std::endl;

		//Check for Disk Full
		//if (errno == EDQUOT) return DISK_FULL;

		return DIRECTORYERR;
	}

	std::string xmlFilePath(m_xmlFile);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << std::endl;
	int iPos = xmlFilePath.find_last_of("/");
	std::string cmdDestFolderxml = m_finalSourceFolder + xmlFilePath.substr(iPos+1);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << std::endl;

	if (CCH_Util::CopyFile(xmlFilePath.c_str(), cmdDestFolderxml.c_str(),false, 4096) == false)
	{
		//error
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << "\n" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << "\n" << std::endl;
		//Store error nr of copyFile
		int errCopy = errno;

		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		//Check for Disk Full
		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;

	}

	return EXECUTED;
}


 int FIXS_CCH_Installer::installIplb ()
{
	std::string sourceSubFolder(""); //used to store PACKAGE_ID + "_" + PACKAGE_RSTATE
	std::string relativePath(""); // read from LM_REL_PATH; it is the relative path where the boot file is placed
	std::string bootFile("");
	std::string cmdTempFolderrelativePath("");
	std::string cmdTempFolderLm("");
	std::string cmdDestFolderLm("");
	std::string softwareRstate("");
	std::string softwarePrNumber("");

	// get properties
	xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PACKAGE_ID);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_ID not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	        }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property PACKAGE_ID not set!" << std::endl;

		return XMLERROR;
	}
	else
	{
		sourceSubFolder = (*it).second;
	}
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;

	//xmlPropertiesMap_t::iterator
	it = m_xmlProperties.find(PACKAGE_RSTATE);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_RSTATE not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	        }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property PACKAGE_RSTATE not set!" << std::endl;

		return XMLERROR;
	}
	else
	{
		//set source final subfolder using xml-metafile field
		sourceSubFolder = sourceSubFolder + "_" + (*it).second;
		m_installedPackage = sourceSubFolder;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	//xmlPropertiesMap_t::iterator
	it = m_xmlProperties.find(LM_REL_PATH);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_REL_PATH not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	        }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property LM_REL_PATH not set!" << std::endl;

		return XMLERROR;
	}
	else
	{
		//Get relativePath of the file IPLB.tar from the xml-metafile field
		relativePath = (*it).second;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	//xmlPropertiesMap_t::iterator
	it = m_xmlProperties.find(LM_BOOTFILE_NAME);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_BOOTFILE_NAME not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	        }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property LM_BOOTFILE_NAME not set!" << std::endl;

		return XMLERROR;
	}
	else
	{
		//Get relativePath of the file IPLB.tar from the xml-metafile field
		bootFile = (*it).second;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	it = m_xmlProperties.find(LM_PRODUCT_NUMBER);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_PRODUCT_NUMBER not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	        }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property LM_PRODUCT_NUMBER not set!" << std::endl;

		return XMLERROR;
	}
	else
        {
		 m_installedProduct = (*it).second;
		 softwarePrNumber = (*it).second;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	it = m_xmlProperties.find(LM_RSTATE);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_RSTATE not set!");
			_trace->ACS_TRA_event(1, tmpStr);
	        }

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Error. xml property LM_RSTATE not set!" << std::endl;

		return XMLERROR;
	}
	else
	{
		m_installedProduct = m_installedProduct + "_" + (*it).second;
		softwareRstate = (*it).second;
	}

	//check if the package is marked as faulty
	if (!FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(m_installedPackage))
	{

		if (FIXS_CCH_SoftwareMap::getInstance()->isContainerInstalled(m_installedPackage))
		{
			//package already installed

			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] Package %s is already installed.",
						__FUNCTION__, __LINE__, m_installedPackage.c_str());
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				//		FIXS_CCH_Logger::createInstance()->log(tmpStr, FIXS_CCH_Logger::Error);
				std::cout << "DBG: " << tmpStr << std::endl;
			}
			return PACKAGE_ALREADY_INSTALLED;
		}

		if (FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(m_installedProduct, CCH_Util::IPLB))
		{
			//product already installed i.e CXC
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,
						"[%s@%d] Product %s is already installed. Wrong package %s",
						__FUNCTION__, __LINE__, m_installedProduct.c_str(), m_installedPackage.c_str());
				std::cout << "DBG: " << tmpStr << std::endl;
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
				//		FIXS_CCH_Logger::createInstance()->log(tmpStr, FIXS_CCH_Logger::Error);
			}
			return PACKAGE_ALREADY_INSTALLED;
		}
	}
	//------------------------
	//Set enviroment variables
	//------------------------
	// build m_finalSourceFolder directory

	std::string iplbBasePath = FIXS_CCH_DiskHandler::getIplbFolder();

	m_finalSourceFolder = iplbBasePath + sourceSubFolder + "/";
    cmdTempFolderrelativePath = CCH_Util::TEMP_FOLDER + "/" + relativePath + "/";
	cmdTempFolderLm = CCH_Util::TEMP_FOLDER + "/" + relativePath + "/" + bootFile;
	cmdDestFolderLm = m_finalSourceFolder + bootFile;

	//file name extracted from package
	m_fileInPackage = bootFile;

	//debugging
	std::cout << "DBG: m_finalSourceFolder " << m_finalSourceFolder << std::endl;
	std::cout << "DBG: cmdTempFolderrelativePath " << cmdTempFolderrelativePath << std::endl;
        std::cout << "DBG: cmdTempFolderLm" << cmdTempFolderLm << std::endl;
        std::cout << "DBG: cmdDestFolderLm" << cmdDestFolderLm << std::endl;

	if(!CCH_Util::PathFileExists(cmdTempFolderLm.c_str()))
	{
		{ //trace

			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] cannot find the boot file: %s in relative path: %s",
				__FUNCTION__, __LINE__, bootFile.c_str(), relativePath.c_str());
			if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
	    }
		return XMLERROR;
	}

	//verify if the final destination folder exist. If not, we create it.
	if(!CCH_Util::PathFileExists(m_finalSourceFolder.c_str()))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CreateDirectory(" << m_finalSourceFolder.c_str() << ")" << std::endl;

		int retValue = ACS_APGCC::create_directories(m_finalSourceFolder.c_str(),ACCESSPERMS);

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " create ok:" << retValue << std::endl;

	}
	else
	{
		std::string traceStr = "Directory ";
		traceStr += m_finalSourceFolder;
		traceStr += ", already present.";
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceStr.c_str());
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << traceStr.c_str() << std::endl;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_finalSourceFolder = " << m_finalSourceFolder.c_str() << std::endl;
	if (!CCH_Util::CopyFolder(cmdTempFolderrelativePath.c_str(), m_finalSourceFolder.c_str()))
	{
		//Store error nr of copyFile
		int errCopy = errno;

		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		//Check for Disk Full
		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;
	}
	else
	{
		std::string iplbProxyDir = "/cluster/etc/ap/ext/IPLB";
		std::string iplbProxySymbLink = "/data/apz/data/IPLB/IPLBProxy";

		// create IPLB Proxy Directory
		if (!CCH_Util::PathFileExists(iplbProxyDir.c_str()))
		{
			//Directory doesn't exist, create it...
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__	<< " creating iplbProxyDirr = " << iplbProxyDir.c_str() << std::endl;
			int retValue = ACS_APGCC::create_directories(iplbProxyDir.c_str(),ACCESSPERMS);
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " CREATE " << std::endl;
			if (retValue == -1)
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__<< " error = " << std::endl;
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH - can't create IPLB Proxy folder: %s",iplbProxyDir.c_str());
				FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			}
			else
			{
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH - create IPLB Proxy folder: %s",iplbProxyDir.c_str());
				FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			}
		}
		else
		{
			char tmpStr[512] = {0};
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " IPLB Proxy folder already exist" << std::endl;
			snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH -  IPLB Proxy folder already exist: %s",iplbProxyDir.c_str());
			FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
		}


		//createSymbolicLink for IPLB Proxy
		if (CCH_Util::isSymbolicLinkExists(iplbProxySymbLink.c_str()))
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " iplbProxySymbLink already exists" << std::endl;
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH -  iplbProxySymbLink already exists: %s",iplbProxySymbLink.c_str());
			FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
		}
		else
		{
			//Create it
			int result = symlink(iplbProxyDir.c_str(), iplbProxySymbLink.c_str());
			if (result)
			{
				std::cout << "DBG: " << result << __FUNCTION__ << "@" << __LINE__ << " error in iplbProxySymbLink" << std::endl;
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH - can't create IPLB Proxy link: %s", iplbProxySymbLink.c_str());
				FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			}
			else
			{
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__	<< " iplbProxySymbLink is OK!!" << std::endl;
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "FIXS_CCH - create IPLB Proxy link OK: %s" ,iplbProxySymbLink.c_str());
				FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_DEBUG);
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			}
		}
	}

	std::string xmlFilePath(m_xmlFile);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << std::endl;
	int iPos = xmlFilePath.find_last_of("/");
	std::string cmdDestFolderxml = m_finalSourceFolder + xmlFilePath.substr(iPos+1);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << std::endl;

	if (CCH_Util::CopyFile(xmlFilePath.c_str(), cmdDestFolderxml.c_str(),false, 4096) == false)
	{
		//error
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << "\n" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << "\n" << std::endl;
		//Store error nr of copyFile
		int errCopy = errno;

		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		//Check for Disk Full
		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;

	}

	return EXECUTED;
}

 int FIXS_CCH_Installer::installEpb1 ()
{

	std::string sourceSubFolder(""); //used to store PACKAGE_ID + "_" + PACKAGE_RSTATE
	std::string realPath(""); // read from LM_REL_PATH; it is the relative path where the boot file is placed
	std::string bootFile(""); // EPB1.tar
	std::string cmdTempFolderLm("");
	std::string cmdDestFolderLm("");
	std::string softwareRstate(""); // LM_RSTATE
	std::string softwarePrNumber(""); // read from LM_PRODUCT_NUMBER, used to store the product number

	// get properties
	xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PACKAGE_ID);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_ID not set!");
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
	    }
		return XMLERROR;
	}
	else
	{
		sourceSubFolder = (*it).second;
	}

	//xmlPropertiesMap_t::iterator
	it = m_xmlProperties.find(PACKAGE_RSTATE);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_RSTATE not set!");
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		}
		return XMLERROR;
	}
	else
	{
		//set source final subfolder using xml-metafile field
		sourceSubFolder = sourceSubFolder + "_" + (*it).second;  //example id="CXP9014804" rstate="R5J01" sourceSubFolder = CXP9014804_R5J01
		m_installedPackage = sourceSubFolder;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	//xmlPropertiesMap_t::iterator
	it = m_xmlProperties.find(LM_REL_PATH);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_REL_PATH not set!");
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
	    }
		return XMLERROR;
	}
	else
	{
		//Get realpath of the file SCX.tar from the xml-metafile field
		realPath = (*it).second; // example SCXB-LM_CXC138894/scxb_lm-R5J01/priv/img
	    //replace(realPath.begin(), realPath.end(), '/', '\\' );
	}

	//xmlPropertiesMap_t::iterator
	it = m_xmlProperties.find(LM_BOOTFILE_NAME);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_BOOTFILE_NAME not set!");
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
	    }
		return XMLERROR;
	}
	else
	{
		//Get realpath of the file EPB1.tar from the xml-metafile field
		bootFile = (*it).second; // example EPB1.tar
	}

	it = m_xmlProperties.find(LM_PRODUCT_NUMBER);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_PRODUCT_NUMBER not set!");
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
	    }
		return XMLERROR;
	}
	else
	{
		 m_installedProduct = (*it).second;
		 softwarePrNumber = (*it).second;
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	it = m_xmlProperties.find(LM_RSTATE);
	if (it == m_xmlProperties.end())
	{
		//error xml property not set!
		if (_trace->ACS_TRA_ON())
		{ //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_RSTATE not set!");
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
	    }
		return XMLERROR;
	}
	else
	{
		m_installedProduct = m_installedProduct + "_" + (*it).second;
		softwareRstate = (*it).second;
	}

	//check if the package is marked as faulty
	if (!FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(m_installedPackage))
	{
		if (FIXS_CCH_SoftwareMap::getInstance()->isContainerInstalled(m_installedPackage))
		{
			//package already installed
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Package already installed.");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return PACKAGE_ALREADY_INSTALLED;
		}

		if (FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(m_installedProduct, CCH_Util::EPB1))
		{
			//product already installed i.e CXC
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Product already installed.");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return PACKAGE_ALREADY_INSTALLED;
		}
	}

	// check the no.of Load Modules installed for EPB1
	if(FIXS_CCH_SoftwareMap::getInstance()->getEpb1Size() >= 10 )
	{
		return MAXIMUM_LOAD_MODULES_INSTALLED;
	}

	//Set enviroment variables, create final destination folder and copy file
	// build m_finalSourceFolder directory
	std::string epb1BasePath = FIXS_CCH_DiskHandler::getEpb1Folder();
	m_finalSourceFolder = epb1BasePath + sourceSubFolder + "/"; //example K:/EPB1/DATA/CXP9014804_R5J01/

	// build "K:/TEMP_INSTALL/IPT_LM_138894/EPB1.tar"
	cmdTempFolderLm = CCH_Util::TEMP_FOLDER + "/" + realPath + "/" + bootFile;
	// build "K:/EPB1/DATA/CXP9014804_R5J01/EPB1.tar" file name
	cmdDestFolderLm = m_finalSourceFolder + bootFile;

	//file name extracted from package
	m_fileInPackage = bootFile;

    //verify if the final destination folder exist. If not, we create it.
	if(!CCH_Util::PathFileExists(m_finalSourceFolder.c_str()))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CreateDirectory(" << m_finalSourceFolder.c_str() << ")" << std::endl;
		int retValue = ACS_APGCC::create_directories(m_finalSourceFolder.c_str(),ACCESSPERMS);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " create ok:" << retValue << std::endl;
	}
	else
	{
		std::string traceStr = "Directory ";
		traceStr += m_finalSourceFolder;
		traceStr += ", already present.";

		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceStr.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceStr.c_str(),LOG_LEVEL_ERROR);
	}

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_finalSourceFolder = " << m_finalSourceFolder.c_str() << std::endl;
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " cmdTempFolderLm.c_str() " << cmdTempFolderLm.c_str() <<" cmdDestFolderLm "<<cmdDestFolderLm.c_str()<<std::endl;

	if (CCH_Util::CopyFile(cmdTempFolderLm.c_str(), cmdDestFolderLm.c_str(), false, 4096) == false)
	{
		int errCopy = errno;

		//error
		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;
	}

	std::string xmlFilePath(m_xmlFile);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << std::endl;
	int iPos = xmlFilePath.find_last_of("/");
	std::string cmdDestFolderxml = m_finalSourceFolder + xmlFilePath.substr(iPos+1);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << std::endl;

	if (CCH_Util::CopyFile(xmlFilePath.c_str(), cmdDestFolderxml.c_str(),false, 4096) == false)
	{
		//error
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << "\n" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << "\n" << std::endl;
		//Store error nr of copyFile
		int errCopy = errno;

		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		//Check for Disk Full
		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;

	}

	return EXECUTED;

}

 int FIXS_CCH_Installer::installCmx()
  {

 	 std::string sourceSubFolder(""); //used to store PACKAGE_ID + "_" + PACKAGE_RSTATE
 	 std::string realPath(""); // read from LM_REL_PATH; it is the relative path where the boot file is placed
 	 std::string bootFile(""); // EPB1.tar
 	 std::string cmdTempFolderLm("");
 	 std::string cmdDestFolderLm("");
 	 std::string softwareRstate(""); // LM_RSTATE
 	 std::string softwarePrNumber(""); // read from LM_PRODUCT_NUMBER, used to store the product number
 	 std::string swVerRel(""); // read from VERSION_REL, used to store the load module package release version

 	 // get properties
 	 xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PACKAGE_ID);
 	 if (it == m_xmlProperties.end())
 	 {
 		 //error xml property not set!
 		 { //trace
 			 char tmpStr[512] = {0};
 			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_ID not set!");
 			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
 			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
 		 }
 		 return XMLERROR;
 	 }
 	 else
 	 {
 		 sourceSubFolder = (*it).second;
 	 }

 	 //xmlPropertiesMap_t::iterator
 	 it = m_xmlProperties.find(PACKAGE_RSTATE);
 	 if (it == m_xmlProperties.end())
 	 {
 		 //error xml property not set!
 		 { //trace
 			 char tmpStr[512] = {0};
 			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_RSTATE not set!");
 			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
 			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
 		 }
 		 return XMLERROR;
 	 }
 	 else
 	 {
 		 //set source final subfolder using xml-metafile field
 		 sourceSubFolder = sourceSubFolder + "_" + (*it).second;  //example id="CXP9014804" rstate="R5J01" sourceSubFolder = CXP9014804_R5J01
 		 m_installedPackage = sourceSubFolder;
 	 }

 	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
 	 //xmlPropertiesMap_t::iterator
 	 it = m_xmlProperties.find(LM_REL_PATH);
 	 if (it == m_xmlProperties.end())
 	 {
 		 //error xml property not set!
 		 { //trace
 			 char tmpStr[512] = {0};
 			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_REL_PATH not set!");
 			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
 			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
 		 }
 		 return XMLERROR;
 	 }
 	 else
 	 {
 		 //Get realpath of the file SCX.tar from the xml-metafile field
 		 realPath = (*it).second; // example SCXB-LM_CXC138894/scxb_lm-R5J01/priv/img
 		 //replace(realPath.begin(), realPath.end(), '/', '\\' );
 	 }

 	 //xmlPropertiesMap_t::iterator
 	 it = m_xmlProperties.find(LM_BOOTFILE_NAME);
 	 if (it == m_xmlProperties.end())
 	 {
 		 //error xml property not set!
 		 { //trace
 			 char tmpStr[512] = {0};
 			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_BOOTFILE_NAME not set!");
 			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
 			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
 		 }
 		 return XMLERROR;
 	 }
 	 else
 	 {
 		 //Get realpath of the file EPB1.tar from the xml-metafile field
 		 bootFile = (*it).second; // example EPB1.tar
 	 }

 	 it = m_xmlProperties.find(LM_PRODUCT_NUMBER);
 	 if (it == m_xmlProperties.end())
 	 {
 		 //error xml property not set!
 		 { //trace
 			 char tmpStr[512] = {0};
 			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_PRODUCT_NUMBER not set!");
 			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
 			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
 		 }
 		 return XMLERROR;
 	 }
 	 else
 	 {
 		 m_installedProduct = (*it).second;
 		 softwarePrNumber = (*it).second;
 	 }

 	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
 	 it = m_xmlProperties.find(LM_RSTATE);
 	 if (it == m_xmlProperties.end())
 	 {
 		 //error xml property not set!
 		 if (_trace->ACS_TRA_ON())
 		 { //trace
 			 char tmpStr[512] = {0};
 			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_RSTATE not set!");
 			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
 			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
 		 }
 		 return XMLERROR;
 	 }
 	 else
 	 {
 		 m_installedProduct = m_installedProduct + "_" + (*it).second;
 		 softwareRstate = (*it).second;
 	 }

	 
 	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
 	 it = m_xmlProperties.find(VERSION_REL);
 	 if (it == m_xmlProperties.end())
 	 {
 		 //error xml property not set!
 		 if (_trace->ACS_TRA_ON())
 		 { //trace
			char tmpStr[512] = {0};
			snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property VERSION_REL not set!");
			if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
 		 }
 		 return XMLERROR;
 	 }
 	 else
 	 {
 		 swVerRel = (*it).second;
 	 }

	 if(swVerRel.compare("1.0") != 0)
	 {
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		it = m_xmlProperties.find(LM_SW_VER_TYPE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_SW_VER_TYPE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			std::string temp = (*it).second;
			m_swVerType = atoi(temp.c_str());
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmx version = " << m_swVerType << std::endl;
		}
	 }

 	 //check if the package is marked as faulty
 	 if (!FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(m_installedPackage))
 	 {
 		 if (FIXS_CCH_SoftwareMap::getInstance()->isContainerInstalled(m_installedPackage))
 		 {
 			 //package already installed
 			 { //trace
 				 char tmpStr[512] = {0};
 				 snprintf(tmpStr, sizeof(tmpStr) - 1, "Package already installed.");
 				 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
 				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
 			 }
 			 return PACKAGE_ALREADY_INSTALLED;
 		 }

 		 if (FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(m_installedProduct, CCH_Util::CMXB))
 		 {
 			 //product already installed i.e CXC
 			 { //trace
 				 char tmpStr[512] = {0};
 				 snprintf(tmpStr, sizeof(tmpStr) - 1, "Product already installed.");
 				 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
 				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
 			 }
 			 return PACKAGE_ALREADY_INSTALLED;
 		 }
 	 }

 	 //Set enviroment variables, create final destination folder and copy file
 	 // build m_finalSourceFolder directory
 	 std::string cmxBasePath = FIXS_CCH_DiskHandler::getCmxFolder();
 	 m_finalSourceFolder = cmxBasePath + sourceSubFolder + "/";

 	 // build "K:/TEMP_INSTALL/IPT_LM_138894/EPB1.tar"
 	 cmdTempFolderLm = CCH_Util::TEMP_FOLDER + "/" + realPath + "/" + bootFile;
 	 // build "K:/EPB1/DATA/CXP9014804_R5J01/EPB1.tar" file name
 	 cmdDestFolderLm = m_finalSourceFolder + bootFile;

 	 //file name extracted from package
 	 m_fileInPackage = bootFile;

 	 //verify if the final destination folder exist. If not, we create it.
 	 if(!CCH_Util::PathFileExists(m_finalSourceFolder.c_str()))
 	 {
 		 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CreateDirectory(" << m_finalSourceFolder.c_str() << ")" << std::endl;
 		 int retValue = ACS_APGCC::create_directories(m_finalSourceFolder.c_str(),ACCESSPERMS);
 		 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " create ok:" << retValue << std::endl;
 	 }
 	 else
 	 {
 		 std::string traceStr = "Directory ";
 		 traceStr += m_finalSourceFolder;
 		 traceStr += ", already present.";

 		 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceStr.c_str());
 		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceStr.c_str(),LOG_LEVEL_ERROR);
 	 }

 	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_finalSourceFolder = " << m_finalSourceFolder.c_str() << std::endl;
 	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " cmdTempFolderLm.c_str() " << cmdTempFolderLm.c_str() <<" cmdDestFolderLm "<<cmdDestFolderLm.c_str()<<std::endl;

 	 if (CCH_Util::CopyFile(cmdTempFolderLm.c_str(), cmdDestFolderLm.c_str(), false, 4096) == false)
 	 {
 		 //Store error nr of copyFile
 		 int errCopy = errno;

 		 CCH_Util::removeFolder(m_finalSourceFolder.c_str());
 		 char tmpStr[512] = {0};
 		 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
 		 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
 		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

 		 //Check for Disk Full
 		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;
 	 }

	std::string xmlFilePath(m_xmlFile);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << std::endl;
	int iPos = xmlFilePath.find_last_of("/");
	std::string cmdDestFolderxml = m_finalSourceFolder + xmlFilePath.substr(iPos+1);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << std::endl;

	if (CCH_Util::CopyFile(xmlFilePath.c_str(), cmdDestFolderxml.c_str(),false, 4096) == false)
	{
		//error
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << "\n" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << "\n" << std::endl;
		//Store error nr of copyFile
		int errCopy = errno;

		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		//Check for Disk Full
		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;

	}

 	 return EXECUTED;

  }


 int FIXS_CCH_Installer::installEvoEt ()
 {

	 std::string sourceSubFolder(""); //used to store PACKAGE_ID + "_" + PACKAGE_RSTATE
	 std::string realPath(""); // read from LM_REL_PATH; it is the relative path where the boot file is placed
	 std::string bootFile(""); // EPB1.tar
	 std::string cmdTempFolderLm("");
	 std::string cmdDestFolderLm("");
	 std::string softwareRstate(""); // LM_RSTATE
	 std::string softwarePrNumber(""); // read from LM_PRODUCT_NUMBER, used to store the product number

	 // get properties
	 xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PACKAGE_ID);
	 if (it == m_xmlProperties.end())
	 {
		 //error xml property not set!
		 { //trace
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_ID not set!");
			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		 }
		 return XMLERROR;
	 }
	 else
	 {
		 sourceSubFolder = (*it).second;
	 }

	 //xmlPropertiesMap_t::iterator
	 it = m_xmlProperties.find(PACKAGE_RSTATE);
	 if (it == m_xmlProperties.end())
	 {
		 //error xml property not set!
		 { //trace
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_RSTATE not set!");
			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		 }
		 return XMLERROR;
	 }
	 else
	 {
		 //set source final subfolder using xml-metafile field
		 sourceSubFolder = sourceSubFolder + "_" + (*it).second;  //example id="CXP9014804" rstate="R5J01" sourceSubFolder = CXP9014804_R5J01
		 m_installedPackage = sourceSubFolder;
	 }

	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	 //xmlPropertiesMap_t::iterator
	 it = m_xmlProperties.find(LM_REL_PATH);
	 if (it == m_xmlProperties.end())
	 {
		 //error xml property not set!
		 { //trace
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_REL_PATH not set!");
			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		 }
		 return XMLERROR;
	 }
	 else
	 {
		 //Get realpath of the file SCX.tar from the xml-metafile field
		 realPath = (*it).second; // example SCXB-LM_CXC138894/scxb_lm-R5J01/priv/img
		 //replace(realPath.begin(), realPath.end(), '/', '\\' );
	 }

	 //xmlPropertiesMap_t::iterator
	 it = m_xmlProperties.find(LM_BOOTFILE_NAME);
	 if (it == m_xmlProperties.end())
	 {
		 //error xml property not set!
		 { //trace
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_BOOTFILE_NAME not set!");
			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		 }
		 return XMLERROR;
	 }
	 else
	 {
		 //Get realpath of the file EPB1.tar from the xml-metafile field
		 bootFile = (*it).second; // example EPB1.tar
	 }

	 it = m_xmlProperties.find(LM_PRODUCT_NUMBER);
	 if (it == m_xmlProperties.end())
	 {
		 //error xml property not set!
		 { //trace
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_PRODUCT_NUMBER not set!");
			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		 }
		 return XMLERROR;
	 }
	 else
	 {
		 m_installedProduct = (*it).second;
		 softwarePrNumber = (*it).second;
	 }

	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
	 it = m_xmlProperties.find(LM_RSTATE);
	 if (it == m_xmlProperties.end())
	 {
		 //error xml property not set!
		 if (_trace->ACS_TRA_ON())
		 { //trace
			 char tmpStr[512] = {0};
			 snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_RSTATE not set!");
			 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
			 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
		 }
		 return XMLERROR;
	 }
	 else
	 {
		 m_installedProduct = m_installedProduct + "_" + (*it).second;
		 softwareRstate = (*it).second;
	 }

	 //check if the package is marked as faulty
	 if (!FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(m_installedPackage))
	 {
		 if (FIXS_CCH_SoftwareMap::getInstance()->isContainerInstalled(m_installedPackage))
		 {
			 //package already installed
			 { //trace
				 char tmpStr[512] = {0};
				 snprintf(tmpStr, sizeof(tmpStr) - 1, "Package already installed.");
				 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			 }
			 return PACKAGE_ALREADY_INSTALLED;
		 }

		 if (FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(m_installedProduct, CCH_Util::EVOET))
		 {
			 //product already installed i.e CXC
			 { //trace
				 char tmpStr[512] = {0};
				 snprintf(tmpStr, sizeof(tmpStr) - 1, "Product already installed.");
				 if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			 }
			 return PACKAGE_ALREADY_INSTALLED;
		 }
	 }

	 // check the no.of Load Modules installed for EVOET
	 if(FIXS_CCH_SoftwareMap::getInstance()->getEvoEtSize() >= 10 )
	 {
		 return MAXIMUM_LOAD_MODULES_INSTALLED;
	 }

	 //Set enviroment variables, create final destination folder and copy file
	 // build m_finalSourceFolder directory
	 std::string evoEtBasePath = FIXS_CCH_DiskHandler::getEvoEtFolder();
	 m_finalSourceFolder = evoEtBasePath + sourceSubFolder + "/";

	 // build "K:/TEMP_INSTALL/IPT_LM_138894/EPB1.tar"
	 cmdTempFolderLm = CCH_Util::TEMP_FOLDER + "/" + realPath + "/" + bootFile;
	 // build "K:/EPB1/DATA/CXP9014804_R5J01/EPB1.tar" file name
	 cmdDestFolderLm = m_finalSourceFolder + bootFile;

	 //file name extracted from package
	 m_fileInPackage = bootFile;

	 //verify if the final destination folder exist. If not, we create it.
	 if(!CCH_Util::PathFileExists(m_finalSourceFolder.c_str()))
	 {
		 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CreateDirectory(" << m_finalSourceFolder.c_str() << ")" << std::endl;
		 int retValue = ACS_APGCC::create_directories(m_finalSourceFolder.c_str(),ACCESSPERMS);
		 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " create ok:" << retValue << std::endl;
	 }
	 else
	 {
		 std::string traceStr = "Directory ";
		 traceStr += m_finalSourceFolder;
		 traceStr += ", already present.";

		 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceStr.c_str());
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceStr.c_str(),LOG_LEVEL_ERROR);
	 }

	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " m_finalSourceFolder = " << m_finalSourceFolder.c_str() << std::endl;
	 std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " cmdTempFolderLm.c_str() " << cmdTempFolderLm.c_str() <<" cmdDestFolderLm "<<cmdDestFolderLm.c_str()<<std::endl;

	 if (CCH_Util::CopyFile(cmdTempFolderLm.c_str(), cmdDestFolderLm.c_str(), false, 4096) == false)
	 {
		 //Store error nr of copyFile
		 int errCopy = errno;

		 CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		 char tmpStr[512] = {0};
		 snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		 if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		 if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		 //Check for Disk Full
		 return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;
	 }

	std::string xmlFilePath(m_xmlFile);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << std::endl;
	int iPos = xmlFilePath.find_last_of("/");
	std::string cmdDestFolderxml = m_finalSourceFolder + xmlFilePath.substr(iPos+1);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << std::endl;

	if (CCH_Util::CopyFile(xmlFilePath.c_str(), cmdDestFolderxml.c_str(),false, 4096) == false)
	{
		//error
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << "\n" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << "\n" << std::endl;
		//Store error nr of copyFile
		int errCopy = errno;

		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		//Check for Disk Full
		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;

	}
	 return EXECUTED;

 }
int FIXS_CCH_Installer::installSmx ()
{
	std::string sourceSubFolder("");
	std::string realPath("");
	std::string smxFile("");
	std::string cmdTempFolderSmx("");
	std::string cmdDestFolderSmx("");
	std::string softwareRstate("");
	std::string softwarePrNumber("");
	//check of new version has to be checked when actual package is obtained
	bool newVersion = false;

	xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PRODUCT_ID);
	if (it == m_xmlProperties.end())
	{
		xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PACKAGE_ID);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_ID or PRODUCT_ID not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			sourceSubFolder = (*it).second;
			newVersion = true;
		}
	}
	else
	{
		sourceSubFolder = (*it).second;
	}

	//xmlPropertiesMap_t::iterator
	if (!newVersion)
	{
		it = m_xmlProperties.find(PRODUCT_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PRODUCT_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//set source final subfolder using xml-metafile field
			sourceSubFolder = sourceSubFolder + "_" + (*it).second;  //example id="CXP9014804" rstate="R5J01" sourceSubFolder = CXP9014804_R5J01
			m_installedPackage = sourceSubFolder;
		}


		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(FILE_RELPATH);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_REALPATH not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			//Get realpath of the file SMX.tar from the xml-metafile field
			realPath = (*it).second; // example SMXB-LM_CXC138894/smxb_lm-R5J01/priv/img
			//replace(realPath.begin(), realPath.end(), '/', '\\' );
		}

		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(FILE_NAME);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_NAME not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			//Get realpath of the file SMX.tar from the xml-metafile field
			smxFile = (*it).second; // example SMX.tar
		}

		it = m_xmlProperties.find(FILE_PROVIDER_PRODNR);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_PROVIDER_PRODNR not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			m_installedProduct = (*it).second;
			softwarePrNumber = (*it).second;
		}


		it = m_xmlProperties.find(FILE_PROVIDER_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_PROVIDER_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			m_installedProduct = m_installedProduct + "_" + (*it).second;
			softwareRstate = (*it).second;
		}

	}
	else
	{
		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(PACKAGE_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//set source final subfolder using xml-metafile field
			sourceSubFolder = sourceSubFolder + "_" + (*it).second;  //example id="CXP9014804" rstate="R5J01" sourceSubFolder = CXP9014804_R5J01
			m_installedPackage = sourceSubFolder;
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(LM_REL_PATH);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_REL_PATH not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//Get realpath of the file SMX.tar from the xml-metafile field
			realPath = (*it).second; // example SMXB-LM_CXC138894/smxb_lm-R5J01/priv/img
			//replace(realPath.begin(), realPath.end(), '/', '\\' );
		}
		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(LM_BOOTFILE_NAME);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_BOOTFILE_NAME not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//Get realpath of the file SMX.tar from the xml-metafile field
			smxFile = (*it).second; // example SMX.tar
		}
		it = m_xmlProperties.find(LM_PRODUCT_NUMBER);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_PRODUCT_NUMBER not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			m_installedProduct = (*it).second;
			softwarePrNumber = (*it).second;
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		it = m_xmlProperties.find(LM_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			m_installedProduct = m_installedProduct + "_" + (*it).second;
			softwareRstate = (*it).second;
		}
	}

	//check if the package is marked as faulty
	if (!FIXS_CCH_SoftwareMap::getInstance()->isFaultyPackage(m_installedPackage))
	{
		if (FIXS_CCH_SoftwareMap::getInstance()->isContainerInstalled(m_installedPackage))
		{
			//package already installed
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Package %s is already installed.",__FUNCTION__, __LINE__, m_installedPackage.c_str());
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}
			return PACKAGE_ALREADY_INSTALLED;
		}
		if (FIXS_CCH_SoftwareMap::getInstance()->isProductInstalled(m_installedProduct, CCH_Util::SMXB))
		{
			//product already installed i.e CXC
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1,"[%s@%d] Product %s is already installed. Wrong package %s",__FUNCTION__, __LINE__, m_installedProduct.c_str(), m_installedPackage.c_str());
				std::cout << "DBG: " << tmpStr << std::endl;
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
			}
			return PACKAGE_ALREADY_INSTALLED;
		}
	}

	//Set enviroment variables, create final destination folder and copy file
	// build m_finalSourceFolder directory
	std::string smxBasePath = FIXS_CCH_DiskHandler::getSmxFolder();
	m_finalSourceFolder = smxBasePath + sourceSubFolder + "/"; //example K:/IPT/DATA/CXP9014804_R5J01/
	
    // build "K:/TEMP_INSTALL/SMXB-LM_CXC138894/smxb_lm-R5J01/priv/img/SMX.tar"
	cmdTempFolderSmx = CCH_Util::TEMP_FOLDER + "/" + realPath + smxFile;
	// build "K:/APZ/DATA/BOOT/SMX/CXP9014804_R5J01/SMX.tar" file name
	cmdDestFolderSmx = m_finalSourceFolder + smxFile;
	//file name extracted from package
	m_fileInPackage = smxFile;

    //verify if the final destination folder exist. If not, we create it.

	if(!CCH_Util::PathFileExists(m_finalSourceFolder.c_str()))
	{
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " CreateDirectory(" << m_finalSourceFolder.c_str() << ")" << std::endl;
		int retValue = ACS_APGCC::create_directories(m_finalSourceFolder.c_str(),ACCESSPERMS);
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << " create ok:" << retValue << std::endl;
	}
	else
	{
		std::string traceStr = "Directory ";
		traceStr += m_finalSourceFolder;
		traceStr += ", already present.";
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceStr.c_str());
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceStr.c_str(),LOG_LEVEL_ERROR);
	}

	if (CCH_Util::CopyFile(cmdTempFolderSmx.c_str(), cmdDestFolderSmx.c_str(), false, 4096) == false)
	{
		//error
		//Svuoto directory K:/APZ/DATA/BOOT/SMX/CXP9014804_R5J01/ da tutti i file (smx.tar e conf.ini)
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdTempFolderSmx: " << cmdTempFolderSmx.c_str() << "\n" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderSmx: " << cmdDestFolderSmx.c_str() << "\n" << std::endl;
		//Store error nr of copyFile
		int errCopy = errno;

		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		//Check for Disk Full
		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;

	}

	std::string xmlFilePath(m_xmlFile);
	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << std::endl;
	int iPos = xmlFilePath.find_last_of("/");
	std::string cmdDestFolderxml = m_finalSourceFolder + xmlFilePath.substr(iPos+1);

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << std::endl;

	if (CCH_Util::CopyFile(xmlFilePath.c_str(), cmdDestFolderxml.c_str(),false, 4096) == false)
	{
		//error
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmdDestFolderxml: " << cmdDestFolderxml.c_str() << "\n" << std::endl;
		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "xmlFilePath: " << xmlFilePath.c_str() << "\n" << std::endl;
		//Store error nr of copyFile
		int errCopy = errno;

		CCH_Util::removeFolder(m_finalSourceFolder.c_str());
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "[%s@%d] CopyFile failed.", __FUNCTION__, __LINE__);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, tmpStr);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		//Check for Disk Full
		return (errCopy == EDQUOT)? DISK_FULL: DIRECTORYERR;

	}

	return EXECUTED;
}

int FIXS_CCH_Installer::fetchPackageInfo()
{	
	std::string softwareRstate("");
	std::string softwarePrNumber("");
	std::string sourceSubFolder("");
	std::string realPath("");
	std::string File("");
	std::string swVerRel("");                             // TR HW71825  

	bool newVersion = false;

	xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PRODUCT_ID);
	if (it == m_xmlProperties.end())
	{
		xmlPropertiesMap_t::iterator it = m_xmlProperties.find(PACKAGE_ID);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_ID or PRODUCT_ID not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			sourceSubFolder = (*it).second;
			newVersion = true;
		}
	}
	else
	{
		sourceSubFolder = (*it).second;
	}

	//xmlPropertiesMap_t::iterator
	if (!newVersion)
	{
		it = m_xmlProperties.find(PRODUCT_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PRODUCT_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//set source final subfolder using xml-metafile field
			sourceSubFolder = sourceSubFolder + "_" + (*it).second;  //example id="CXP9014804" rstate="R5J01" sourceSubFolder = CXP9014804_R5J01
			m_installedPackage = sourceSubFolder;
		}


		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(FILE_RELPATH);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_REALPATH not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			//Get realpath of the file SCX.tar from the xml-metafile field
			realPath = (*it).second; // example SCXB-LM_CXC138894/scxb_lm-R5J01/priv/img
			//replace(realPath.begin(), realPath.end(), '/', '\\' );
		}
		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(FILE_NAME);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_NAME not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			//Get realpath of the file SCX.tar from the xml-metafile field
			File = (*it).second; // example SCX.tar
		}
		it = m_xmlProperties.find(FILE_PROVIDER_PRODNR);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_PROVIDER_PRODNR not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			m_installedProduct = (*it).second;
			softwarePrNumber = (*it).second;
		}


		it = m_xmlProperties.find(FILE_PROVIDER_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property FILE_PROVIDER_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;

		}
		else
		{
			m_installedProduct = m_installedProduct + "_" + (*it).second;
			softwareRstate = (*it).second;
		}

	}
	else
	{
		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(PACKAGE_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property PACKAGE_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//set source final subfolder using xml-metafile field
			sourceSubFolder = sourceSubFolder + "_" + (*it).second;  //example id="CXP9014804" rstate="R5J01" sourceSubFolder = CXP9014804_R5J01
			m_installedPackage = sourceSubFolder;
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(LM_REL_PATH);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_REL_PATH not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//Get realpath of the file SCX.tar from the xml-metafile field
			realPath = (*it).second; // example SCXB-LM_CXC138894/scxb_lm-R5J01/priv/img
			//replace(realPath.begin(), realPath.end(), '/', '\\' );
		}

		//xmlPropertiesMap_t::iterator
		it = m_xmlProperties.find(LM_BOOTFILE_NAME);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_BOOTFILE_NAME not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			//Get realpath of the file EPB1.tar from the xml-metafile field
			File = (*it).second; // example EPB1.tar
		}

		it = m_xmlProperties.find(LM_PRODUCT_NUMBER);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_PRODUCT_NUMBER not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			m_installedProduct = (*it).second;
			softwarePrNumber = (*it).second;
		}

		std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
		it = m_xmlProperties.find(LM_RSTATE);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_RSTATE not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			m_installedProduct = m_installedProduct + "_" + (*it).second;
			softwareRstate = (*it).second;
		}

		//TR HW71825  BEGIN
		
		it = m_xmlProperties.find(VERSION_REL);
		if (it == m_xmlProperties.end())
		{
			//error xml property not set!
			if (_trace->ACS_TRA_ON())
			{ //trace
				char tmpStr[512] = {0};
				snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property VERSION_REL not set!");
				if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
			}
			return XMLERROR;
		}
		else
		{
			swVerRel = (*it).second;
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmx sw version = " << swVerRel << std::endl;
		}
		
		if(swVerRel.compare("1.0") != 0)
		{
			std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << std::endl;
			it = m_xmlProperties.find(LM_SW_VER_TYPE);
			if (it == m_xmlProperties.end())
			{
				//error xml property not set!
				if (_trace->ACS_TRA_ON())
				{ //trace
					char tmpStr[512] = {0};
					snprintf(tmpStr, sizeof(tmpStr) - 1, "Error. xml property LM_SW_VER_TYPE not set!");
					if (_trace->ACS_TRA_ON())_trace->ACS_TRA_event(1, tmpStr);
					if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
				}
				return XMLERROR;
			}
			else
			{
				std::string temp = (*it).second;
				m_swVerType = atoi(temp.c_str());
				std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "cmx version type = " << m_swVerType << std::endl;
			}
		}
		
		//TR HW71825  END
	}

	m_fileInPackage = File;

	return 0;
}
bool FIXS_CCH_Installer::SetXmlFileAfterRestore(std::string xmlPath)
{
	char SwpDirSpec[1000] = {0};
	snprintf(SwpDirSpec, sizeof(SwpDirSpec) - 1, "%s", xmlPath.c_str());

	std::cout << "DBG: " << __FUNCTION__ << "@" << __LINE__ << "Finding .xml file!" << std::endl;

	char type[10]={0};
	int dimErr=0;
	std::vector<std::string> xmlfiles;
	sprintf(type,"%s",".xml");

	dimErr=CCH_Util::findFile(SwpDirSpec,type,xmlfiles);

	if(xmlfiles.size()>0)
	{
		snprintf(m_xmlFile, sizeof(m_xmlFile) - 1, "%s", xmlfiles[0].c_str());
	}
	else
	{
		char traceChar[512] = {0};
		snprintf(traceChar, sizeof(traceChar) - 1, "[%s@%d] Wrong Installation file. Cannot find any xml file at \"%s\"", __FUNCTION__, __LINE__, SwpDirSpec);
		if (_trace->ACS_TRA_ON()) _trace->ACS_TRA_event(1, traceChar);
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write(traceChar,LOG_LEVEL_ERROR);
		return false;
	}
	return initializeParser();
}	
