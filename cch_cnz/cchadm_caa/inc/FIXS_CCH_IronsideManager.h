/*
 * FIXS_CCH_IronsideManager.h
 *
 *  Created on: Apr 30, 2013
 *      Author: xgunsar
 */

#ifndef FIXS_CCH_IronsideManager_h
#define FIXS_CCH_IronsideManager_h 1

#include "ace/OS.h"
#include "acs_nclib_library.h"
#include "Client_Server_Interface.h"
#include "FIXS_CCH_IronsideUdpCommand.h"
#include "ACS_TRA_Logging.h"
#include "FIXS_CCH_DiskHandler.h"
#include "FIXS_CCH_IMM_Util.h"
#include <map>

class acs_nclib_session;

class FIXS_CCH_IronsideManager{

public:
	FIXS_CCH_IronsideManager(uint32_t dmxc_address1, uint32_t dmxc_address2);
	~FIXS_CCH_IronsideManager();


	static inline void initialize () { acs_nclib_library::init(); }
	static inline void shutdown () { acs_nclib_library::exit(); }


	int block_board(
			uint32_t board_magazine,
			int32_t board_slot);
	int dblock_board(
			uint32_t board_magazine,
			int32_t board_slot);

	int reset_board(
			uint32_t board_magazine,
			int32_t board_slot,
			IMM_Util::BoardResetType type);
	
	int reset_board(
			uint32_t board_magazine,
			int32_t board_slot);
			
	int isBoardblocked(
			uint32_t board_magazine,
			int32_t board_slot);

        int isBoarddblocked(
                        uint32_t board_magazine,
                        int32_t board_slot);

	int setUpgradeType(
			uint32_t board_magazine,
			int32_t board_slot,
			IMM_Util::BoardUpgradeType type );

	int setBootType(
			uint32_t board_magazine,
			int32_t board_slot,
			IMM_Util::BootType type);

	int getGprData(std::string &gprDataValue,uint32_t board_magazine,
			int32_t board_slot, std::string gprId);

	int get_board_Mialed_status(std::string shelfId ,int32_t slot,std::string & led_status);

	int set_board_Mialed(std::string shelfId ,int32_t slot,std::string led_status);

	int get_led_type(std::string shelfId ,int32_t slot , std::string &ledType);
	
	int setboardConfiguration(std::string shelfId, int32_t slot, IMM_Util::BootType bootType);


private:

	ACS_TRA_Logging *FIXS_CCH_logging;
	static std::string tenantName;
	std::map<int,std::string> _shelf_addr_id_map;
	int shelf_addr_to_physical_addr(const char* shelfaddr);

	int get_shelfId_from_address(int ph_addr, std::string& shelfId);
	int get_shelfId_from_address(const char* addr, std::string& shelfId);

	int get_shelfId_from_address_loop(int ph_addr, std::string& shelfId);
	int get_shelfId_from_address_loop(const char* addr, std::string& shelfId);
	int get_boardConfiguration(std::string shelfId, int32_t slot, std::string &boardConfigurationStr);
	int setGprData(std::string shelfId,int32_t slot,std::string gprid,int gprdata[]);
	void stringtohex(std::string strval,long int &hexval);
	uint32_t _dmxc_addresses[2]; 
	int uint32_to_ip_format (char (& value_ip) [16], uint32_t value);


};

#endif /* FIXS_CCH_IronsideManager_h */
