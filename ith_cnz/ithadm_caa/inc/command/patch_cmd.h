/*
 * smx_patch_cmd.h
 *
 *  Created on: Oct 12, 2016
 *      Author: estevol
 */

#ifndef ITHCMD_CAA_SRC_SMX_PATCH_CMD_H_
#define ITHCMD_CAA_SRC_SMX_PATCH_CMD_H_


#include <string.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <stdint.h>


namespace patch_cmd {
	enum commandExitCodes {
		NO_ERROR = 0,
		INCORRECT_USAGE = 1,
		INTERNAL_ERROR=2,
		INVALID_FILENAME,
		INVALID_OPERATION,
		INVALID_PLANE,
		INVALID_SLOT,
		INVALID_MAGAZINE,
		FAILED_OPERATION,
		PARSING_ERROR,
		BOARD_NOT_FOUND,
		FILE_NOT_FOUND,
		SNMP_OPEN_SESS_ERROR,
		SNMP_PARSE_OID_ERROR,
		SNMP_RESPONSE_ERROR,
		SNMP_STATUS_TIMEOUT,
		SNMP_STATUS_ERROR,
		SNMP_PDU_CREATE_ERROR,
		NC_SEND_RCV_FAILED,
		NC_SESSION_OPEN_FAILED,
		NC_SESSION_CLOSE_FAILED
	};

	enum OperationType {
		UNKNOWN_OP = -1,
		SNMP_SET = 0,
		NETCONF_EDIT_CONFIG
	};

	enum ActionOnError
	{
		ABORT = 0,
		CONTINUE
	};

	enum Plane {
		UNDEFINED_PLANE = -1,
		CONTROL_PLANE = 0,
		TRANSPORT_PLANE = 1
		};

	static const std::string SNMPSET_OPERATION = "SNMPSET";
	static const std::string NCEDITCFG_OPERATION = "NETCONFEDITCFG";
	static const std::string TRY_SNMPSET_OPERATION = "TRY_SNMPSET";
	static const std::string TRY_NCEDITCFG_OPERATION = "TRY_NETCONFEDITCFG";

	static const std::string CONTROL_PLANE_LABEL = "LS";
	static const std::string TRANSPORT_PLANE_LABEL = "HS";

	static const std::string SNMP_COMMUNITY = "NETMAN";
	static const uint32_t SNMP_RETRIES = 1;
	static const uint64_t SNMP_TIMEOUT_US = 3000000;

	static const std::string NETCONF_USER = "advanced";
	static const std::string NETCONF_PASSWD = "ett,30";
	static const uint32_t NETCONF_PORT = 22;
	static const uint32_t NETCONF_TIMEOUT_MS = 3000;


	int ip_format_to_uint32 (uint32_t & value, const char * value_ip);

	int uint32_to_ip_format (const uint32_t value, std::string& value_ip);
}




class patch_operation
{
public:
	patch_operation(std::string operation_id, patch_cmd::OperationType operation_type, patch_cmd::ActionOnError action_on_error, std::vector<std::string> params, uint32_t control_ip[2], uint32_t transport_ip[2] ):
		_operation_id(operation_id),
		_operation_type(operation_type),
		_action_on_error(action_on_error),
		_params(params),
		_switch_ip(),
		_plane(patch_cmd::UNDEFINED_PLANE),
		_error_message()

	{
		memcpy(_switch_ip[patch_cmd::CONTROL_PLANE], control_ip, 2 * sizeof(uint32_t));
		memcpy(_switch_ip[patch_cmd::TRANSPORT_PLANE], transport_ip, 2 * sizeof(uint32_t));

	};

	~patch_operation()
	{

	};

	int parse_parameters();
	int execute();

	inline std::string get_error_message() {return _error_message;};

	inline std::string get_operation_id() {return _operation_id;};

private:
	std::string _operation_id;
	patch_cmd::OperationType _operation_type;
	patch_cmd::ActionOnError _action_on_error;
	std::vector<std::string> _params;
	uint32_t _switch_ip[2][2];
	int _plane;
	std::string _error_message;

	int execute_snmp_operation();
	int execute_netconf_operation();

	int parse_snmp_parameters();
	int parse_netconf_parameters();

	int parse_plane(std::string& plane_label);

	int snmp_set(const char* community, uint32_t ip_addr,
			const char* usrOID,  char type, const char *value, std::string& error_message);

	int snmp_set(const char* community, const char* peername, const char* usrOID,
			char type, const char *value, std::string& error_message);

	int netconf_editconfig(uint32_t ipaddr, const char* config, std::string& operation_errors);

};

class smx_patch_cmd
{
public:
	smx_patch_cmd(int argc, char * argv []);
	smx_patch_cmd(const char * file_name);
	virtual ~smx_patch_cmd();

	int parse();
	int execute();

	void error_msg(int err_code, std::ostream& out);
	void print_usage(std::ostream& out);
private:
	int _argc;
	std::vector<std::string> _argv;
	std::vector<patch_operation> _operations;
	std::string _last_execution_error;
	std::string _last_operation_id;

	int get_switch_ip(const uint32_t mag, const uint16_t slot, uint32_t (& control_ip) [2], uint32_t (& transport_ip) [2]);

	int parse_tweak_file(const std::string& file_name, uint32_t control_ip[2], uint32_t transport_ip[2] );

	void split(const std::string &s, char delim, std::vector<std::string> &elems);

};

#endif /* ITHCMD_CAA_SRC_SMX_PATCH_CMD_H_ */
