/*
 * fixs_ith_cmdoptionparser.h
 *
 *  Created on: Jan 22, 2016
 *      Author: xludesi
 */

#ifndef ITHADM_CAA_INC_FIXS_ITH_CMDOPTIONPARSER_H_
#define ITHADM_CAA_INC_FIXS_ITH_CMDOPTIONPARSER_H_

#include <unistd.h>
#include <getopt.h>

/** @class fixs_ith_cmdoptionparser fixs_ith_cmdoptionparser.h
 *	@brief fixs_ith_cmdoptionparser class
 *	@author
 *	@date 2016-01-22
 *
 *	Utility class to parse command line
 */
class fixs_ith_cmdoptionparser {
	//==============//
	// Constructors //
	//==============//
public:
	/** @brief fixs_ith_cmdoptionparser constructor
	 */
	inline fixs_ith_cmdoptionparser (int argc, char * const argv []) : _argc(argc), _argv(argv), _noha(0) {}

private:
	fixs_ith_cmdoptionparser (const fixs_ith_cmdoptionparser & rhs);

	//============//
	// Destructor //
	//============//
public:
	/** @brief fixs_ith_cmdoptionparser Destructor
	 */
	inline ~fixs_ith_cmdoptionparser () {}

	//=================//
	// Field Accessors //
	//=================//
public:
	inline int argc () const { return _argc; }

	inline const char * const * argv () const { return _argv; }

	inline int noha () const { return _noha; }

	const char * program_name () const;

	//===========//
	// Functions //
	//===========//
public:
	int parse ();

	//===========//
	// Operators //
	//===========//
private:
	fixs_ith_cmdoptionparser & operator= (const fixs_ith_cmdoptionparser & rhs);

	//========//
	// Fields //
	//========//
private:
	int _argc;
	char * const * _argv;
	int _noha;

	static const char * _program_name;
	static struct option _long_options [];
};

#endif /* ITHADM_CAA_INC_FIXS_ITH_CMDOPTIONPARSER_H_ */
