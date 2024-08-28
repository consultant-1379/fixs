/*
 * smxpatchexec.cpp
 *
 *  Created on: Oct 12, 2016
 *      Author: estevol
 */
#include "command/patch_cmd.h"
#include <iostream>

int main (int argc, char * argv [])
{
	smx_patch_cmd cmd(argc, argv);

	int ret_val = cmd.parse();

	if (ret_val != patch_cmd::NO_ERROR)
	{
		cmd.error_msg(ret_val, std::cout);
		return ret_val;
	}


	ret_val = cmd.execute();
	if (ret_val != patch_cmd::NO_ERROR)
	{
		cmd.error_msg(ret_val, std::cout);
		return ret_val;
	}

	std::cout << std::endl;
	std::cout << "Command executed" << std::endl;
	std::cout << std::endl;
	return patch_cmd::NO_ERROR;
}



