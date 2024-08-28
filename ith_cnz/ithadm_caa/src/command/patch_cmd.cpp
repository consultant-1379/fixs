/*
 * smx_patch_cmd.cpp
 *
 *  Created on: Oct 12, 2016
 *      Author: estevol
 */

#include "command/patch_cmd.h"

#include <fstream>
#include <sstream>
#include <arpa/inet.h>
#include <sys/stat.h>


#include <ACS_CS_API.h>
#include <ACS_CS_API_Set.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/mib_api.h>
#include <net-snmp/library/mib.h>


#include "acs_nclib_session.h"
#include "acs_nclib_message.h"
#include "acs_nclib_hello.h"
#include "acs_nclib_rpc.h"
#include "acs_nclib_factory.h"
#include "acs_nclib_library.h"


int patch_operation::execute()
{
	switch(_operation_type)
	{
		case patch_cmd::SNMP_SET:
			return execute_snmp_operation();
		case patch_cmd::NETCONF_EDIT_CONFIG:
			return execute_netconf_operation();
		default:
			return patch_cmd::INVALID_OPERATION;
	}
}


int patch_operation::parse_parameters()
{
	switch(_operation_type)
	{
	case patch_cmd::SNMP_SET:
		return parse_snmp_parameters();
	case patch_cmd::NETCONF_EDIT_CONFIG:
		return parse_netconf_parameters();
	default:
		return patch_cmd::INVALID_OPERATION;
	}
}


int patch_operation::parse_plane(std::string& plane_label)
{
	if (plane_label.compare(patch_cmd::CONTROL_PLANE_LABEL) == 0)
	{
		_plane = patch_cmd::CONTROL_PLANE;
	}
	else if (plane_label.compare(patch_cmd::TRANSPORT_PLANE_LABEL) == 0)
	{
		_plane = patch_cmd::TRANSPORT_PLANE;
	}
	else
	{
		return patch_cmd::INVALID_PLANE;
	}

	return patch_cmd::NO_ERROR;
}

int patch_operation::parse_netconf_parameters()
{
	/*
	 * _params[0] 	PLANE
	 * _params[1]	CONFIG
	 */
	if (_params.size() < 2)
	{
		return patch_cmd::PARSING_ERROR;
	}

	return parse_plane(_params[0]);
}


int patch_operation::parse_snmp_parameters()
{
	/*
	 * _params[0] 	PLANE
	 * _params[1]	OID
	 * _params[2]	TYPE
	 * _params[3]	VALUE
	 */

	if (_params.size() < 4)
	{
		return patch_cmd::PARSING_ERROR;
	}

	if (_params[2].size() > 1)
	{
		return patch_cmd::PARSING_ERROR;
	}


	return parse_plane(_params[0]);
}

int patch_operation::execute_netconf_operation()
{
	std::string config;
	for (unsigned int i = 1; i < _params.size(); ++i)
	{

		config += _params[i] + std::string(" ");
	}
	config.assign(config.substr(0, config.size() - 1));

	unsigned int i = 0;
	int op_result = patch_cmd::INTERNAL_ERROR;

	do
	{
		_error_message.clear();
		op_result = netconf_editconfig(_switch_ip[_plane][i], config.c_str(), _error_message);
		++i;
	} while (op_result != patch_cmd::NO_ERROR && i < 2);

	return (patch_cmd::ABORT == _action_on_error)? op_result: patch_cmd::NO_ERROR;
}

int patch_operation::execute_snmp_operation()
{
	std::string oid = _params[1];
	std::string type = _params[2];
	std::string value;
	for (unsigned int i = 3; i< _params.size(); ++i)
	{
		value += _params[i] + std::string(" ");
	}
	value.assign(value.substr(0, value.size() - 1));

	unsigned int i = 0;
	int op_result = patch_cmd::INTERNAL_ERROR;
	do
	{
		_error_message.clear();
		op_result = snmp_set(patch_cmd::SNMP_COMMUNITY.c_str(), _switch_ip[_plane][i], oid.c_str(), type.c_str()[0], value.c_str(), _error_message);
		++i;
	} while (op_result != patch_cmd::NO_ERROR && i < 2);

	return (patch_cmd::ABORT == _action_on_error)? op_result: patch_cmd::NO_ERROR;
}


int patch_operation::snmp_set(const char* community, uint32_t ip_addr,
		const char* usrOID,  char type, const char *value, std::string& error_message)
{
	std::string peerName;
	patch_cmd::uint32_to_ip_format(ip_addr, peerName);

	return snmp_set(community, peerName.c_str(), usrOID, type, value, error_message);
}

int patch_operation::snmp_set(const char* community, const char* peername,
		const char* usrOID,  char type, const char *value, std::string& error_message)
{
	/*
	 TYPE: one of i, u, t, a, o, s, x, d, b
	 i: INTEGER, u: unsigned INTEGER, t: TIMETICKS, a: IPADDRESS
	 o: OBJID, s: STRING, x: HEX STRING, d: DECIMAL STRING, b: BITS
	 U: unsigned int64, I: signed int64, F: float, D: double
	 */

	snmp_session snmpSession;
	snmp_session *snmpSessionHandler;

	int setResult = patch_cmd::NO_ERROR;
	struct snmp_pdu *snmpPDU;
	struct snmp_pdu *snmpResp = NULL;

	snmp_sess_init(&snmpSession);

	snmpSession.version = SNMP_VERSION_2c;
	snmpSession.community = (u_char*) strdup(community);
	snmpSession.retries = patch_cmd::SNMP_RETRIES;
	snmpSession.timeout = patch_cmd::SNMP_TIMEOUT_US;
	snmpSession.community_len = strlen((const char *) snmpSession.community);
	snmpSession.peername = strdup(peername);

	snmpSessionHandler = snmp_open(&snmpSession);

	if (snmpSessionHandler == NULL)
	{
		error_message.assign("SNMP-SET: Failed to open SNMP session");
		return patch_cmd::SNMP_OPEN_SESS_ERROR;
	}

	//snmpPDU = snmp_pdu_create(SNMP_MSG_SET);

	oid snmp_oid[MAX_OID_LEN];
	size_t snmp_oid_len = MAX_OID_LEN;

	if (!read_objid(usrOID, snmp_oid, &snmp_oid_len))
	{
		snmp_close(snmpSessionHandler);
		error_message.assign("SNMP-SET: Invalid OID");
		return patch_cmd::SNMP_PARSE_OID_ERROR;
	}

	if ((snmpPDU = snmp_pdu_create(SNMP_MSG_SET)) != NULL)
	{

		if (snmp_add_var(snmpPDU, snmp_oid, snmp_oid_len, type, value))
		{
			snmp_close(snmpSessionHandler);
			error_message.assign("SNMP-SET: Invalid OID");
			return patch_cmd::SNMP_PARSE_OID_ERROR;
		}

		// send pdu...
		int res = snmp_synch_response(snmpSessionHandler, snmpPDU, &snmpResp);

		switch (res)
		{
		case STAT_SUCCESS:
			//Check the response
			if (snmpResp->errstat != SNMP_ERR_NOERROR)
			{
				std::stringstream error_stream;
				error_stream << "SNMP-SET-Response error: " << snmp_errstring(snmpResp->errstat) << std::endl;
				error_message.assign(error_stream.str());
				setResult = patch_cmd::SNMP_RESPONSE_ERROR;
			}
			break;

		case STAT_TIMEOUT:
			error_message.assign("SNMP-SET: Timeout");
			setResult = patch_cmd::SNMP_STATUS_TIMEOUT;
			break;

		case STAT_ERROR:
			error_message.assign("SNMP-SET: Failed");
			setResult = patch_cmd::SNMP_STATUS_ERROR;
			break;

		} //switch(res)
	}
	else //if ((snmpPDU = snmp_pdu_create(SNMP_MSG_SET)) != NULL)
	{
		error_message.assign("SNMP-SET: Internal Error");
		setResult = patch_cmd::SNMP_PDU_CREATE_ERROR;
	}

	if (snmpResp)
	{
		snmp_free_pdu(snmpResp);
		snmpResp = NULL;
	}

	snmp_close(snmpSessionHandler);

	return setResult;
}

int patch_operation::netconf_editconfig(uint32_t ipaddr, const char* config, std::string& operation_errors)
{
	acs_nclib_library::init();

	acs_nclib_session session(acs_nclib::SSH);

	session.setPassword(patch_cmd::NETCONF_PASSWD.c_str());

	if (session.open(ipaddr, patch_cmd::NETCONF_PORT, acs_nclib::USER_AUTH_PASSWORD, patch_cmd::NETCONF_USER.c_str()) == 0)
	{
		acs_nclib_rpc* rpc = acs_nclib_factory::create_rpc(acs_nclib::OP_EDITCONFIG);
		rpc->set_config(config);

		acs_nclib_rpc_reply* reply = 0;
		int result = 0;

		if ((result = session.send_sync(rpc, reply, patch_cmd::NETCONF_TIMEOUT_MS)) == acs_nclib::ERR_NO_ERRORS)
		{
			if (reply)
			{
				if (reply->get_reply_type() != acs_nclib::REPLY_OK)
				{
					std::vector<acs_nclib_rpc_error*> errors;
					reply->get_error_list(errors);

					for (std::vector<acs_nclib_rpc_error*>::iterator it = errors.begin(); it != errors.end(); ++it)
					{
						std::stringstream error_stream;
						error_stream << "Error type " <<  (*it)->get_error_type() << ". Error message: " << (*it)->get_error_message() << std::endl;
						operation_errors += error_stream.str();
					}

					acs_nclib_factory::dereference(errors);
					return patch_cmd::NC_SEND_RCV_FAILED;
				}

				acs_nclib_factory::dereference(reply);
			}
			else
			{
				operation_errors.assign("NETCONF EDITCONFIG: No Reply Received");
				return patch_cmd::NC_SEND_RCV_FAILED;
			}
		}
		else
		{
			operation_errors.assign("NETCONF EDITCONFIG: No Reply Received");
			return patch_cmd::NC_SEND_RCV_FAILED;
		}

		if (rpc)
			acs_nclib_factory::dereference(rpc);

		if (session.close() != 0)
		{
			operation_errors.assign("NETCONF EDITCONFIG: Failed to close NETCONF session");
			return patch_cmd::NC_SESSION_CLOSE_FAILED;
		}
	}
	else
	{
		operation_errors.assign("NETCONF EDITCONFIG: Failed to open NETCONF session");
		return patch_cmd::NC_SESSION_OPEN_FAILED;
	}

	acs_nclib_library::exit();
	return patch_cmd::NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

smx_patch_cmd::smx_patch_cmd(int argc, char * argv []):
	_argc(argc),
	_argv(),
	_operations()
{
	for (int i = 0; i < _argc; ++i)
	{
		_argv.push_back(argv[i]);
	}
}

smx_patch_cmd::smx_patch_cmd(const char * file_name):
		_argc(2),
		_argv(),
		_operations()
{
	_argv.push_back("");
	_argv.push_back(file_name);
}

smx_patch_cmd::~smx_patch_cmd()
{

}

int smx_patch_cmd::parse()
{
	if (_argc != 2)
	{
		return patch_cmd::INCORRECT_USAGE;
	}

	std::string full_file_name(_argv[1]);

	//Check if file exists
	struct stat buffer;
	if (0 != stat (full_file_name.c_str(), &buffer))
	{
		return patch_cmd::FILE_NOT_FOUND;
	}

	//Validate fileName format
	std::vector<std::string> elements;
	split(full_file_name, '/', elements);

	std::string file_name = elements[elements.size() - 1];

	elements.clear();
	split(file_name, '_', elements);

	if (elements.size() != 2)
	{
		return patch_cmd::INVALID_FILENAME;
	}

	//Validate magazine and slot
	std::string magazine = elements[0];
	std::string slot_str = elements[1];

	uint16_t slot = 0xffff;

	std::istringstream slot_stream(slot_str);
	slot_stream >> slot;

	if (slot_stream.fail() || ( 0 != slot && 25 != slot) )
	{
		return patch_cmd::INVALID_SLOT;
	}

	uint32_t mag = 0;
	if ( patch_cmd::NO_ERROR != patch_cmd::ip_format_to_uint32(mag, magazine.c_str()) ||
			0 != (mag & 0x00ff0000))
	{
		return patch_cmd::INVALID_MAGAZINE;
	}

	//Fetch SMX IP addresses from CS
	uint32_t control_ip[2] = {0};
	uint32_t transport_ip[2] = {0};

	int get_ip_result = get_switch_ip(mag, slot, control_ip, transport_ip);
	if ( patch_cmd::NO_ERROR != get_ip_result)
	{
		return get_ip_result;
	}

	//Validate file content
	return parse_tweak_file(full_file_name, control_ip, transport_ip);
}

int smx_patch_cmd::get_switch_ip(const uint32_t mag, const uint16_t slot, uint32_t (& control_ip) [2], uint32_t (& transport_ip) [2])
{
	ACS_CS_API_BoardSearch * bs = ACS_CS_API_HWC::createBoardSearchInstance();
	if (!bs) {
		return patch_cmd::INTERNAL_ERROR;
	}

	ACS_CS_API_HWC * hwc = ACS_CS_API::createHWCInstance();
	if (!hwc)
	{ // ERROR: creating HWC CS instance
		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		return patch_cmd::INTERNAL_ERROR;
	}

	// search criteria
	bs->reset();
	bs->setMagazine(mag);
	bs->setSlot(slot);

	// Now I can search
	ACS_CS_API_IdList board_list;
	ACS_CS_API_NS::CS_API_Result cs_call_result = hwc->getBoardIds(board_list, bs);
	if (cs_call_result != ACS_CS_API_NS::Result_Success) { // ERROR: getting board ids from CS
		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		ACS_CS_API::deleteHWCInstance(hwc);
		return patch_cmd::INTERNAL_ERROR;
	}

	// It should be exactly one board
	if (board_list.size() != 1) { // ERROR: no boards found
		ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
		ACS_CS_API::deleteHWCInstance(hwc);
		return patch_cmd::BOARD_NOT_FOUND;
	}

	BoardID board_id = board_list[0];

	int ip_index = (0 == slot)? 0: 1;

	hwc->getIPEthA(control_ip[ip_index], board_id);
	hwc->getIPEthB(control_ip[(ip_index + 1) & 0x01], board_id);

	hwc->getAliasEthA(transport_ip[ip_index], board_id);
	hwc->getAliasEthB(transport_ip[(ip_index + 1) & 0x01], board_id);


	ACS_CS_API_HWC::deleteBoardSearchInstance(bs);
	ACS_CS_API::deleteHWCInstance(hwc);

	return patch_cmd::NO_ERROR;
}


int smx_patch_cmd::execute()
{
	int op_result = patch_cmd::NO_ERROR;
	_last_execution_error.clear();
	_last_operation_id.clear();

	std::vector<patch_operation>::iterator it;

	for ( it = _operations.begin(); it != _operations.end() && patch_cmd::NO_ERROR == op_result; it++)
	{
		op_result = it->execute();

		if (patch_cmd::NO_ERROR != op_result)
		{
			_last_execution_error = it->get_error_message();
			_last_operation_id = it->get_operation_id();
		}
	}

	return op_result;
}

void smx_patch_cmd::error_msg(int err_code, std::ostream& out)
{
	out << std::endl;

	switch(err_code)
	{
	case patch_cmd::NO_ERROR:
		break;
	case patch_cmd::INCORRECT_USAGE:
		out << "Incorrect Usage" << std::endl;
		print_usage(out);
		break;
	case patch_cmd::INVALID_FILENAME:
		out << "Invalid file name" << std::endl;
		print_usage(out);
		break;
	case patch_cmd::INVALID_OPERATION:
		out << "Invalid operation" << std::endl;
		break;
	case patch_cmd::INVALID_PLANE:
		out << "Invalid plane" << std::endl;
		break;
	case patch_cmd::INVALID_SLOT:
		out << "Invalid slot number" << std::endl;
		break;
	case patch_cmd::INVALID_MAGAZINE:
		out << "Invalid magazine number" << std::endl;
		break;
	case patch_cmd::FAILED_OPERATION:
		out << "Operation failed" << std::endl;
		break;
	case patch_cmd::PARSING_ERROR:
		out << "Parsing failed" << std::endl;
		break;
	case patch_cmd::BOARD_NOT_FOUND:
		out << "Hardware not present" << std::endl;
		break;
	case patch_cmd::FILE_NOT_FOUND:
		out << "File not found" << std::endl;
		break;
	case patch_cmd::SNMP_OPEN_SESS_ERROR:
	case patch_cmd::SNMP_PARSE_OID_ERROR:
	case patch_cmd::SNMP_RESPONSE_ERROR:
	case patch_cmd::SNMP_STATUS_TIMEOUT:
	case patch_cmd::SNMP_STATUS_ERROR:
	case patch_cmd::SNMP_PDU_CREATE_ERROR:
		out << "SNMP operation failed" << std::endl;
		break;
	case patch_cmd::NC_SEND_RCV_FAILED:
	case patch_cmd::NC_SESSION_OPEN_FAILED:
	case patch_cmd::NC_SESSION_CLOSE_FAILED:
		out << "NETCONF operation failed" << std::endl;
		break;
	case patch_cmd::INTERNAL_ERROR:
	default:
		out << "Error when executing" << std::endl;
		break;
	}
	out << std::endl;

	//Print operation id
	if (_last_operation_id.size() > 0)
	{
		if (_last_execution_error.size() > 0)
		{
			out << "Execution of operation " << _last_operation_id << " returned an error:" << std::endl;
			out << _last_execution_error << std::endl;
		}
		else
		{
			out << "Error detected on operation " << _last_operation_id << std::endl;
		}
		out << std::endl;
	}
}

void smx_patch_cmd::print_usage(std::ostream& out)
{

	out << "USAGE: smxpatchexec <filename>" << std::endl;
	out << std::endl;
	out << "File Name template: <MAGAZINE>_<SLOT>" << std::endl;
	out << std::endl;
	out << "\t" << "MAGAZINE: SMXB magazine address" << std::endl;
	out << "\t" << "SLOT:     SMXB slot position" << std::endl;
	out << std::endl;
	out << "Operation File Content template: " << std::endl;
	out << std::endl;
	out << "\t<OPERATIONID> SNMPSET <PLANE> <OID> <TYPE> <VALUE>" << std::endl;
	out << "\t<OPERATIONID> TRY_SNMPSET <PLANE> <OID> <TYPE> <VALUE>" << std::endl;
	out << "\t<OPERATIONID> NETCONFEDITCFG <PLANE> <CONFIG>" << std::endl;
	out << "\t<OPERATIONID> TRY_NETCONFEDITCFG <PLANE> <CONFIG>" << std::endl;
	out << std::endl;

	out << "Operations: " << std::endl;
	out << "\tSNMPSET:              Execute SNMP set operation, abort on failure" << std::endl;
	out << "\tTRY_SNMPSET:          Execute SNMP set operation, continue on failure" << std::endl;
	out << "\tNETCONFEDITCFG:       Execute NETCONF edit-config operation, abort on failure" << std::endl;
	out << "\tTRY_NETCONFEDITCFG:   Execute NETCONF edit-config operation, continue on failure" << std::endl;
	out << std::endl;
	out << "Common Parameters: " << std::endl;
	out << "\tOPERATIONID:          Operation Identifier, user defined string to identify the operation" << std::endl;
	out << "\tPLANE:                one of LS, HS\n"
		   "\t                      LS: Low Speed, HS: High Speed"<< std::endl;
	out << std::endl;
	out << "SNMP Parameters: " << std::endl;
	out << "\tOID:   SNMP Object Identifier" << std::endl;
	out << "\tTYPE:  one of i, u, t, a, o, s, x, d, b\n"
	       "\t       i: INTEGER, u: unsigned INTEGER, t: TIMETICKS, a: IPADDRESS\n"
	       "\t       o: OBJID, s: STRING, x: HEX STRING, d: DECIMAL STRING, b: BITS\n"
	       "\t       U: unsigned int64, I: signed int64, F: float, D: double" << std::endl;
	out << "\tVALUE: OID value to set" << std::endl;
	out << std::endl;
	out << "NETCONF Parameters: " << std::endl;
	out << "\tCONFIG: NETCONF configuration to set" << std::endl;
	out << std::endl;

}



int smx_patch_cmd::parse_tweak_file(const std::string& file_name, uint32_t control_ip[2], uint32_t transport_ip[2] )
{
	std::ifstream infile(file_name.c_str());
	std::string line;

	if (!infile)
	{
		return patch_cmd::FILE_NOT_FOUND;
	}

	while (std::getline(infile, line))
	{
		std::vector<std::string> elements;
		split(line, ' ', elements);

		//Empty line. Skip it
		if (elements.size() == 0)
		{
			continue;
		}

		if (elements.size() < 2)
		{
			return patch_cmd::PARSING_ERROR;
		}

		std::string operation_id(elements[0]);
		std::string operation_type(elements[1]);
		std::vector<std::string> operation_params = std::vector<std::string>(elements.begin() + 2, elements.end());

		patch_cmd::OperationType op_type = patch_cmd::UNKNOWN_OP;
		patch_cmd::ActionOnError action = patch_cmd::ABORT;


		if (operation_type.compare(patch_cmd::SNMPSET_OPERATION) == 0)
		{
			op_type = patch_cmd::SNMP_SET;
		}
		else if (operation_type.compare(patch_cmd::TRY_SNMPSET_OPERATION) == 0)
		{
			op_type = patch_cmd::SNMP_SET;
			action = patch_cmd::CONTINUE;
		}
		else if (operation_type.compare(patch_cmd::NCEDITCFG_OPERATION) == 0)
		{
			op_type = patch_cmd::NETCONF_EDIT_CONFIG;
		}
		else if (operation_type.compare(patch_cmd::TRY_NCEDITCFG_OPERATION) == 0)
		{
			op_type = patch_cmd::NETCONF_EDIT_CONFIG;
			action = patch_cmd::CONTINUE;
		}
		else
		{
			_last_operation_id.assign(operation_id);
			return patch_cmd::INVALID_OPERATION;
		}

		patch_operation new_operation(operation_id, op_type, action, operation_params, control_ip, transport_ip);

		int param_parse_res = new_operation.parse_parameters();

		if (patch_cmd::NO_ERROR == param_parse_res)
		{
			_operations.push_back(new_operation);
		}
		else
		{
			_last_operation_id.assign(operation_id);
			return param_parse_res;
		}

	}

	return patch_cmd::NO_ERROR;
}

void smx_patch_cmd::split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
    	if (item.size() > 0)
    	{
    		elems.push_back(item);
    	}
    }
}

namespace patch_cmd {

	int ip_format_to_uint32 (uint32_t & value, const char * value_ip)
	{
		in_addr addr;

		if (::inet_pton(AF_INET, value_ip, &addr) <= 0)
		{ // ERROR: in conversion
			return patch_cmd::INTERNAL_ERROR;
		}

		value = addr.s_addr;

		return patch_cmd::NO_ERROR;
	}


	int uint32_to_ip_format (const uint32_t value, std::string& value_ip)
	{

		char str[INET_ADDRSTRLEN] = {0};

		uint32_t addr = htonl(value);
		if (NULL == inet_ntop(AF_INET, &(addr), str, INET_ADDRSTRLEN) )
		{
			return patch_cmd::INTERNAL_ERROR;
		}

		value_ip = str;

		return patch_cmd::NO_ERROR;
	}
}
