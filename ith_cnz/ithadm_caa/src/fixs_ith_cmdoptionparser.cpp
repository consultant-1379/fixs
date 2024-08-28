/*
 * fixs_ith_cmdoptionparser.cpp
 *
 *  Created on: Jan 22, 2016
 *      Author: xludesi
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "fixs_ith_cmdoptionparser.h"

const char * fixs_ith_cmdoptionparser::_program_name = 0;
struct option fixs_ith_cmdoptionparser::_long_options [] = {
	{"noha", no_argument, 0, 0},
	{0, 0, 0, 0}
};

const char * fixs_ith_cmdoptionparser::program_name () const {
	if (!_program_name) {
#ifdef _GNU_SOURCE
		_program_name = program_invocation_short_name;
#else
		if ((_argc > 0) && _argv && *_argv) {
			const char * last_slash = ::strrchr(*_argv, '/');
			_program_name = last_slash ? last_slash + 1 : *_argv;
		}
#endif
	}

	return _program_name;
}

int fixs_ith_cmdoptionparser::parse () {
	int long_index = 0;
	int opt_code = 0;

	opterr = 0; // Preventing the print of a system error message

	while ((opt_code = ::getopt_long(_argc, _argv, ":c:", _long_options, &long_index)) != -1) {
		if (opt_code == 0) { //A long option detected
			//We accept only an exact match.
			if (::strcmp(_long_options[long_index].name, _argv[::optind - 1] + 2)) { //Unrecognized option
				::fprintf(::stderr, "%s: Unrecognized option '%s'\n", program_name(), _argv[::optind - 1]);
				return -1;
			}

			//--noha option found
			if (_noha++) {
				//--noha option provided more than one time
				::fprintf(::stderr, "%s: multiple instances of option '%s' \n", program_name(), _argv[::optind - 1]);
				return -1;
			}
		} else if (opt_code == '?') {
			//The user provided an option not supported here
			::fprintf(::stderr, "%s: Unsupported option '%s'\n", program_name(), _argv[::optind - 1]);
			return -1;
		} else if (opt_code == ':') {
			//The user missed the option argument. Here we have no such option, but if any in the future, please
			//start the optstring argument in getopt_long call with the ':' character.
			::fprintf(::stderr, "%s: Argument missed for the option '%s'\n", program_name(), _argv[::optind - 1]);
			return -1;
		} else {
			//Other option found but not supported
			::fprintf(::stderr, "%s: Generic error parsing command line options\n", program_name());
			return -1;
		}
	}

	//We must reject the command "fixs_ithd --"
	//Note: if "--" command line argument is found, getopt_long() increments <optind> variable and returns -1.
	if ((::optind > 1) && !::strcmp(_argv[::optind - 1], "--")) {
		::fprintf(::stderr, "%s: Unrecognized option '%s'\n", program_name(), _argv[::optind - 1]);
		return -1;
	}

	//We must check the command has no argument on its line
	if (::optind < _argc) {
		::fprintf(::stderr, "%s: one or more argument found on command line\n", program_name());
		return -1;
	}

	return 0;
}
