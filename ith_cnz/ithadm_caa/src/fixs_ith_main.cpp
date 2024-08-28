#include <stdio.h>

#include "fixs_ith_programconstants.h"
#include "fixs_ith_cmdoptionparser.h"
#include "fixs_ith_snmpmanagercallbacks.h"
#include "fixs_ith_snmpmanager.h"
#include "fixs_ith_server.h"

namespace {
	inline void print_command_usage (const char * program_name) {
		fprintf(stderr, "USAGE: %s [--noha]\n", program_name);
	}
}

/*
 * Application main entry point
 */
int main (int argc, char * argv [])
{
	// Parse command line
	fixs_ith_cmdoptionparser cmd_options(argc, argv);
	if (cmd_options.parse() < 0) {
		print_command_usage(cmd_options.program_name());
		return fixs_ith::PROGRAM_EXIT_BAD_INVOCATION;
	}

	// Initialize the NET-SNMP library
	fixs_ith_snmpmanager::initialize(FIXS_ITH_NETSNMP_INITIALIZER_NAME);

	// Create the object representing the ITH Server
	fixs_ith_server ith_server(cmd_options);

	// Start the ITH server
	const int return_code = ith_server.work();

	// Reset the net-snmp library
	fixs_ith_snmpmanager::shutdown(FIXS_ITH_NETSNMP_INITIALIZER_NAME);

	return return_code;
}
