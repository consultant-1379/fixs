/*
 * fixs_cch_pcDataStorage.h
 *
 *  Created on: Feb 14, 2012
 *      Author: xlucdor
 */

#ifndef FIXS_CCH_PCDATASTORAGE_H_
#define FIXS_CCH_PCDATASTORAGE_H_

//#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <utility>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include "xcountls_Util.h"


class FIXS_CCH_PCDataStorage
{

 public:

	//todo to be replace with correct values !!!
	static const int EXECUTED = 0;
	static const int ERROR_DISK_FULL = -1;

	//Performance Counters keys

	FIXS_CCH_PCDataStorage();

	FIXS_CCH_PCDataStorage(const FIXS_CCH_PCDataStorage &right);

	FIXS_CCH_PCDataStorage & operator=(const FIXS_CCH_PCDataStorage &right);

	static  FIXS_CCH_PCDataStorage * getInstance ();

	int saveBoardCounters (unsigned long magazine, unsigned short slot, XCOUNTLS_Util::PCData data, int no_of_intrface, bool isSMXFlag);

	bool readBoardCounters (unsigned long magazine, unsigned short slot, XCOUNTLS_Util::PCData &data, int (&no_of_intrface)[2], bool isSMX);

	bool deletePCDataStorageFile ();

	bool checkFileExists();

	int saveBaselineHeading();
	int saveBaseline();
	int loadBaseline();

	bool getPath ();

	bool readBaselineHeading(std::string &currentTime);

	int getaddrinfo(const char *nodename, std::string & addressStr);

	bool commitChanges ();

	//removed from saveBoardCounters !
	//this method could be useless
	//int prepareForWriting ();

protected:

private:

	bool createFolder (std::string folder);

	int write (std::string section, std::string key, std::string value);

	bool read (std::string section, std::string key, std::string &value);

	static FIXS_CCH_PCDataStorage* s_instance;

	static std::string m_cfgFileName;

	static std::string m_tmpCFGFileName;

	static std::string m_cfgPath;

	boost::property_tree::ptree pt;

};


#endif /* FIXS_CCH_PCDATASTORAGE_H_ */
