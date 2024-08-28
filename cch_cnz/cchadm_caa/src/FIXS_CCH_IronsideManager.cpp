
#include "FIXS_CCH_IronsideManager.h"
#include "acs_nclib_factory.h"
#include "acs_nclib_session.h"
#include "ace/Guard_T.h"
#include "ace/Recursive_Thread_Mutex.h"
#include <bitset>
#define GPR_ARRAY_SIZE 12
#define CCH_NOTRDY "NOTRDY"
#define CCH_UNSPEC "UNSPEC"

std::string FIXS_CCH_IronsideManager::tenantName = "";

FIXS_CCH_IronsideManager::FIXS_CCH_IronsideManager(uint32_t dmxc_address1,uint32_t dmxc_address2)
{
	_dmxc_addresses[0] = dmxc_address1 ;	
	_dmxc_addresses[1] = dmxc_address2 ;
	FIXS_CCH_logging = FIXS_CCH_DiskHandler::getLogInstance();
}

FIXS_CCH_IronsideManager::~FIXS_CCH_IronsideManager()
{
	FIXS_CCH_logging = 0;
}
int FIXS_CCH_IronsideManager::isBoarddblocked(
                uint32_t board_magazine,
                int32_t board_slot)
{
        int call_result = 0;

        char board_magazine_str [16] = {0};
        int mag_phys_addr = -1;

        //Here we convert Shelf Address in a.b.c.d format
        if (const int call_result = uint32_to_ip_format(board_magazine_str, board_magazine)) // ERROR: while converting
                return call_result;

        mag_phys_addr = shelf_addr_to_physical_addr(board_magazine_str);

        if (mag_phys_addr == -1)
                return CCH_Util::CCH_ERR_IP_ADDRESS_CONVERSION;

        FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0],_dmxc_addresses[1]);
        FIXS_CCH_IronsideUdpCommand*  board_pwr((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));

        //Convert shelf and slot info from integer to char
        char shelf[16] = {0};
        char slot[16] = {0};

        sprintf(shelf,"%d",mag_phys_addr);
        sprintf(slot,"%d",board_slot);

        std::string shelfId("");

        if (get_shelfId_from_address(mag_phys_addr,shelfId) != CCH_Util::CCH_ERR_NO_ERRORS)
        {
           char tmpStr[512] = {0};
           snprintf(tmpStr, sizeof(tmpStr) - 1, "Exiting Method %s@%d, magazine %s ,(physical address %d) not found!)", __FUNCTION__, __LINE__,board_magazine_str, mag_phys_addr);

           if (FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

           return CCH_Util::CCH_ERR_DMX_MAGAZINE_NOT_FOUND;
        }

        cout << "ShelfID = " << shelfId;
        bool pstate = false;
        std::string data;
        char filter[16] = {0};
        snprintf(filter, 13, "BLD:%02d%02dPWR-",atoi(shelfId.c_str()),board_slot);
        call_result = board_pwr->executeUdp(filter, data);
        if (call_result == 0)
        {
                data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
                if (data.compare("ON") == 0)
                {
                         pstate = true;
                }
                else if (data.compare("OFF") == 0)
                {
                         pstate = false;
                }
                else
                {
                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Getting the board status failed",LOG_LEVEL_ERROR);
                        delete board_pwr;
                        board_pwr = NULL;
                        return IRONSIDE_FAILURE;
                }
        }
        else
        {
                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Getting the board status failed",LOG_LEVEL_ERROR);
                delete board_pwr;
                board_pwr = NULL;
                return IRONSIDE_FAILURE;
        }

        if ((pstate == true) )
        {
           printf("Board already deblocked");
           if (FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Board already Blocked or Not Present",LOG_LEVEL_DEBUG);
           delete board_pwr;
           board_pwr = NULL;
           return BOARD_ALREADY_DEBLOCKED; 
        }
        delete board_pwr;
        board_pwr = NULL;
        return CCH_Util::CCH_ERR_NO_ERRORS;
}
int FIXS_CCH_IronsideManager::isBoardblocked(
                uint32_t board_magazine,
                int32_t board_slot)
{

        int call_result = 0;

        char board_magazine_str [16] = {0};
        int mag_phys_addr = -1;

        //Here we convert Shelf Address in a.b.c.d format
        if (const int call_result = uint32_to_ip_format(board_magazine_str, board_magazine)) // ERROR: while converting
                return call_result;

        mag_phys_addr = shelf_addr_to_physical_addr(board_magazine_str);

        if (mag_phys_addr == -1)
                return CCH_Util::CCH_ERR_IP_ADDRESS_CONVERSION;

        FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0],_dmxc_addresses[1]);
        FIXS_CCH_IronsideUdpCommand*  board_led((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));

        //Convert shelf and slot info from integer to char
        char shelf[16] = {0};
        char slot[16] = {0};

        sprintf(shelf,"%d",mag_phys_addr);
        sprintf(slot,"%d",board_slot);

        std::string shelfId("");

        if (get_shelfId_from_address(mag_phys_addr,shelfId) != CCH_Util::CCH_ERR_NO_ERRORS)
        {
           char tmpStr[512] = {0};
           snprintf(tmpStr, sizeof(tmpStr) - 1, "Exiting Method %s@%d, magazine %s ,(physical address %d) not found!)", __FUNCTION__, __LINE__,board_magazine_str, mag_phys_addr);

           if (FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

           return CCH_Util::CCH_ERR_DMX_MAGAZINE_NOT_FOUND;
        }

 	cout << "ShelfID = " << shelfId;
        bool pstate = false;
        std::string data;
	char filter[16] = {0};
        snprintf(filter, 13, "BLD:%02d%02dPWR-",atoi(shelfId.c_str()),board_slot);
	call_result = board_led->executeUdp(filter, data);
        if (call_result == 0)
        {
                data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
                if (data.compare("ON") == 0)
                {
                         pstate = true;
                }
                else if (data.compare("OFF") == 0)
                {
                         pstate = false;
                }
                else
                {
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Getting the board status failed",LOG_LEVEL_ERROR);
			delete board_led;
			board_led = NULL;	
			return IRONSIDE_FAILURE;        	  
                }
        }
        else
        {
                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Getting the board status failed",LOG_LEVEL_ERROR);
                delete board_led;
                board_led = NULL;
                return IRONSIDE_FAILURE;
        }

        if ((pstate == false) )
        {
           printf("Board already blocked");
           if (FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Board already Blocked or Not Present",LOG_LEVEL_DEBUG);
           delete board_led;
           board_led = NULL;
           return BOARD_ALREADY_BLOCKED;
        }
        delete board_led;
        board_led = NULL;
        return CCH_Util::CCH_ERR_NO_ERRORS;
}

int FIXS_CCH_IronsideManager::block_board(
                uint32_t board_magazine,
                int32_t board_slot)
{

        int call_result = 0;

        char board_magazine_str [16] = {0};
        int mag_phys_addr = -1;

        int resp = isBoardblocked(board_magazine,board_slot);

        if (resp!=0)
        {
                return resp;
        }

        //Here we convert Shelf Address in a.b.c.d format
        if (const int call_result = uint32_to_ip_format(board_magazine_str, board_magazine)) // ERROR: while converting
                return call_result;

        mag_phys_addr = shelf_addr_to_physical_addr(board_magazine_str);

        if (mag_phys_addr == -1)
                return CCH_Util::CCH_ERR_IP_ADDRESS_CONVERSION;

        //Convert shelf and slot info from integer to char
        char shelf[16] = {0};
        char slot[16] = {0};

        sprintf(shelf,"%d",mag_phys_addr);
        sprintf(slot,"%d",board_slot);

        std::string shelfId("");

        if (get_shelfId_from_address(mag_phys_addr,shelfId) != CCH_Util::CCH_ERR_NO_ERRORS)
        {
           char tmpStr[512] = {0};
           snprintf(tmpStr, sizeof(tmpStr) - 1, "Exiting Method %s@%d, magazine %s ,(physical address %d) not found!)", __FUNCTION__, __LINE__,board_magazine_str, mag_phys_addr);
           if (FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

           return CCH_Util::CCH_ERR_DMX_MAGAZINE_NOT_FOUND;
        }

        cout << "ShelfID = " << shelfId;

        FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0], _dmxc_addresses[1]);
        FIXS_CCH_IronsideUdpCommand*  board_pwr((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));

        call_result=0;

        std::string pwrstatus = "off";

        char filter[16] = {0};
	
	std::string data;
		
        snprintf(filter, 13, "PWR:%02d%02dOFF-",atoi(shelfId.c_str()),board_slot);
	call_result = board_pwr->executeUdp(filter, data);
        if (call_result == 0)
        {
                data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

                if (data.compare("OK") != 0)
                {
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : blocking board failed",LOG_LEVEL_ERROR);
		        delete board_pwr;
		        board_pwr = NULL;
                        return IRONSIDE_FAILURE;
                }

		call_result = set_board_Mialed(shelfId,board_slot,pwrstatus);
		if (call_result !=0)
		{
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : setting Maintainance LED failed",LOG_LEVEL_ERROR); 	
			delete board_pwr;
			board_pwr = NULL;	
			return IRONSIDE_FAILURE;
		}
        }

        else
        {
                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : blocking board failed",LOG_LEVEL_ERROR);
                delete board_pwr;
                board_pwr = NULL;
                return IRONSIDE_FAILURE;
        }
        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : blocked the board successfully",LOG_LEVEL_DEBUG);
        delete board_pwr;
        board_pwr = NULL;
        return call_result;

}

	
int FIXS_CCH_IronsideManager::set_board_Mialed(std::string shelfId ,int32_t slot,std::string pwr_status)
{
	FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0], _dmxc_addresses[1]);
        FIXS_CCH_IronsideUdpCommand*  board_led((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));
	
	int call_result = 0;
        char filter[16] = {0};
	if (strcmp(pwr_status.c_str(),"off")== 0)
	{
        	snprintf(filter, 13, "MLD:%02d%02dON_-",atoi(shelfId.c_str()),slot);
	}
	else if (strcmp(pwr_status.c_str(),"on")== 0)
	{
		snprintf(filter, 13, "MLD:%02d%02dOFF-",atoi(shelfId.c_str()),slot);
	}
	
        std::string data;

        if ((call_result = board_led->executeUdp(filter, data)) == 0)
        {
                data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

                if (data.compare("OK") != 0)
                {
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Setting the board led status failed",LOG_LEVEL_DEBUG);
                	delete board_led;
                	board_led = NULL;
                	return IRONSIDE_FAILURE; 
                }
        }
        else
        {
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Setting the board led status failed",LOG_LEVEL_DEBUG);
                delete board_led;
                board_led = NULL;
                return IRONSIDE_FAILURE;
        }
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Setting the board led status success",LOG_LEVEL_DEBUG);
	delete board_led;
	board_led = NULL;
	return call_result;
}	
int FIXS_CCH_IronsideManager::dblock_board(
		uint32_t board_magazine,
		int32_t board_slot
                )
{

	int call_result = 0;

	char board_magazine_str [16] = {0};
	int mag_phys_addr = -1;
	int resp = isBoarddblocked(board_magazine,board_slot);

        if (resp!=0)
        {
                return resp;
        }

	//Here we convert Shelf Address in a.b.c.d format
	if (const int call_result = uint32_to_ip_format(board_magazine_str, board_magazine)) // ERROR: while converting
		return call_result;

	mag_phys_addr = shelf_addr_to_physical_addr(board_magazine_str);

	if (mag_phys_addr == -1)
		return CCH_Util::CCH_ERR_IP_ADDRESS_CONVERSION;

	//Convert shelf and slot info from integer to char
	char shelf[16] = {0};
	char slot[16] = {0};

	sprintf(shelf,"%d",mag_phys_addr);
	sprintf(slot,"%d",board_slot);

	std::string shelfId("");
        if (get_shelfId_from_address(mag_phys_addr,shelfId) != CCH_Util::CCH_ERR_NO_ERRORS)
        {
           char tmpStr[512] = {0};
           snprintf(tmpStr, sizeof(tmpStr) - 1, "Exiting Method %s@%d, magazine %s ,(physical address %d) not found!)", __FUNCTION__, __LINE__,board_magazine_str, mag_phys_addr);
           
           if (FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

           return CCH_Util::CCH_ERR_DMX_MAGAZINE_NOT_FOUND; 
        }

	cout << "ShelfID = " << shelfId;

        FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0], _dmxc_addresses[1]);
        FIXS_CCH_IronsideUdpCommand*  board_pwr((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));

        char filter[16] = {0};
	
	std::string pwrstatus ="on";

	std::string data;

        snprintf(filter, 13, "PWR:%02d%02dON_-",atoi(shelfId.c_str()),board_slot);
		call_result = board_pwr->executeUdp(filter, data);
        if (call_result == 0)
        {
                data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

                if (data.compare("OK") != 0)
                {
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Deblocking board failed",LOG_LEVEL_ERROR);
			delete board_pwr;
			board_pwr =NULL;
                        return IRONSIDE_FAILURE;
                }

	        call_result = set_board_Mialed(shelfId,board_slot,pwrstatus);
                if (call_result !=0)
                {
                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : setting Maintainance LED failed",LOG_LEVEL_ERROR);
			delete board_pwr;
			board_pwr = NULL;
                        return IRONSIDE_FAILURE;
                }

        }

        else
        {
                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Deblocking board failed",LOG_LEVEL_ERROR);
                delete board_pwr;
                board_pwr = NULL;
                return IRONSIDE_FAILURE;
        }	

	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Deblocked the board successfully",LOG_LEVEL_DEBUG);
	delete board_pwr;
	board_pwr = NULL;
	return call_result;
}

int FIXS_CCH_IronsideManager::uint32_to_ip_format (char (& value_ip) [16], uint32_t value) {
	// Converting value address into IP dotted-quad format "ddd.ddd.ddd.ddd"
	in_addr addr;
	addr.s_addr = value;
	errno = 0;
	if (!::inet_ntop(AF_INET, &addr, value_ip, sizeof(value_ip))) { // ERROR: in conversion
		value_ip[0] = 0;
		return -1;
	}

	return 0;
}


int FIXS_CCH_IronsideManager::shelf_addr_to_physical_addr(const char* shelfaddr)
{

	std::stringstream ss(shelfaddr);
	std::string item;
	char delim = '.';

	std::vector<std::string> elems;
	while(std::getline(ss, item, delim))
	{
		elems.push_back(item);
	}

	char tmp[32] = {0};
	if (elems.size() == 4)
	{
		int plug0 = atoi(elems.at(0).c_str());
		int plug1 = atoi(elems.at(1).c_str());
		int plug3 = atoi(elems.at(3).c_str());

		sprintf(tmp,"%x%x%x",plug3,plug1,plug0);

	}
	else
		return -1;

	char* endPtr;
	int mag = strtol(tmp, &endPtr,16);

	return mag;

}
int FIXS_CCH_IronsideManager::get_shelfId_from_address(int ph_addr, std::string& shelfId)
{
        std::map<int, std::string>::iterator it;

        shelfId ="";
        int call_result=-1;
        std::string data;
        std::vector<std::string> tokens;
        std::string buf;
	it = _shelf_addr_id_map.find(ph_addr);
	if (it != _shelf_addr_id_map.end())
        {
        	shelfId = it->second;
		call_result =0;				
        }
	else
	{

		FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0], _dmxc_addresses[1]);
		FIXS_CCH_IronsideUdpCommand* comShelf((FIXS_CCH_IronsideUdpCommand*)commandBuilder.make(ironsideBuilder::UDP));
		std::string filter("SHLF--------");

		if (comShelf->executeUdp(filter, data) == 0)
        	{
                	std::stringstream ss(data);

                	while (ss >> buf){
                        	tokens.push_back(buf);
                	}

                	for(uint32_t i = 0; i < tokens.size(); i++)
                	{
                        	std::string shelfid, physadd;
	                        string::size_type pos_start = 0, pos_end;
        	                pos_end = tokens[i] .find(":",pos_start);

                	        if (pos_end != string::npos)
                        	{
                                	shelfid = tokens[i].substr(0, pos_end);
	                                physadd = tokens[i].substr(pos_end+1);
        	                }

				int tmp = atoi(physadd.c_str());
				_shelf_addr_id_map.insert(std::pair<int, std::string>(tmp, shelfid));
                	}

			it = _shelf_addr_id_map.find(ph_addr);
			if (it != _shelf_addr_id_map.end())
			{
				shelfId = it->second;
				call_result =0;
			}

		}
        	delete comShelf;
	        comShelf = NULL;
	}	
        return call_result;
}

/*int FIXS_CCH_IronsideManager::get_shelfId_from_address(const char* shelf_addr, std::string& shelfId)
{

	int mag_phys_addr = -1;

	mag_phys_addr = shelf_addr_to_physical_addr(shelf_addr);

	if (mag_phys_addr == -1)
		return CCH_Util::CCH_ERR_IP_ADDRESS_CONVERSION;

	return get_shelfId_from_address_loop(mag_phys_addr, shelfId);

}*/

int FIXS_CCH_IronsideManager::get_shelfId_from_address(const char* shelf_addr, std::string& shelfId)
{

        int mag_phys_addr = -1;

        mag_phys_addr = shelf_addr_to_physical_addr(shelf_addr);

        if (mag_phys_addr == -1)
                return CCH_Util::CCH_ERR_IP_ADDRESS_CONVERSION;

        return get_shelfId_from_address(mag_phys_addr, shelfId);

}

int FIXS_CCH_IronsideManager::reset_board(
                uint32_t board_magazine,
                int32_t board_slot,
                IMM_Util::BoardResetType type)
{

	int call_result = 0;
	char filter[16] = {0};
	char board_magazine_str [16] = {0};
	int mag_phys_addr = -1;
	
	int resp = isBoardblocked(board_magazine,board_slot);

        if (resp!=0)
        {
           return resp;
        }


	//Here we convert Shelf Address in a.b.c.d format
	if (const int call_result = uint32_to_ip_format(board_magazine_str, board_magazine)) // ERROR: while converting
		return call_result;

	mag_phys_addr = shelf_addr_to_physical_addr(board_magazine_str);

	if (mag_phys_addr == -1)
		return -1;

	FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0], _dmxc_addresses[1]);
	FIXS_CCH_IronsideUdpCommand*  board_gpr((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));

	//Convert shelf and slot info from integer to char
	char shelf[16] = {0};
	char slot[16] = {0};

	sprintf(shelf,"%d",mag_phys_addr);
	sprintf(slot,"%d",board_slot);

	std::string shelfId("");
	// get_shelfId_from_address(mag_phys_addr,shelfId);
	if(get_shelfId_from_address(mag_phys_addr,shelfId) != 0)
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "Exiting Method %s@%d, magazine %s ,(physical address %d) not found!)", __FUNCTION__, __LINE__,board_magazine_str, mag_phys_addr);
		if (FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		return CCH_Util::CCH_ERR_DMX_MAGAZINE_NOT_FOUND;
	}

	/*std::string gprid;
	std::string gprdata;
	if(type == IMM_Util::RESET_NORMAL)
	{
		gprid="512";
		gprdata="255.255.255.255.10.6.255.255";
	}
	else
	{
		gprid="512";
		gprdata="255.255.255.255.2.6.255.255";
	}

	call_result = board_gpr->setGprData(tenant, "",shelfId,slot,gprid,gprdata);
	if (call_result == 0)
	{
		if (board_gpr->execute() != 0)
		{
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Setting GprData failed",LOG_LEVEL_DEBUG);
			delete board_gpr;
			board_gpr = NULL;
			return IRONSIDE_FAILURE;
		}
	}*/

	if((type == IMM_Util::RESET_NORMAL) || (type == IMM_Util::RESET_FORCE_NORMAL))
	{
		call_result = setBootType(board_magazine, board_slot, IMM_Util::FLASH_BOOT);
	}
	else
	{
		call_result = setBootType(board_magazine, board_slot, IMM_Util::NETWORK_BOOT);
	}

	if (call_result != 0)
	{
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Setting boot type failed",LOG_LEVEL_ERROR);
		return call_result;
	}

	//call_result = board_gpr->resetBoard(tenant,"",shelfId,slot);
	snprintf(filter, 13, "RST:%02d%02dHARD",atoi(shelfId.c_str()),board_slot);
	std::string udp_data="";
	if (call_result == 0)
	{
		if (board_gpr->executeUdp(filter,udp_data) == 0)
		{
			udp_data.erase(std::remove(udp_data.begin(), udp_data.end(), '\n'), udp_data.end());

			if (udp_data.compare("OK") != 0)
			{
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Reset of the board with UDP failed",LOG_LEVEL_ERROR);
				delete board_gpr;
				board_gpr = NULL;
				return IRONSIDE_FAILURE;
			}
		}
	}

	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Reset of the board with UDP successful",LOG_LEVEL_DEBUG);
	delete board_gpr;
	board_gpr = NULL;
	return call_result;
}

int FIXS_CCH_IronsideManager::setGprData(std::string shelfId,int32_t slot,std::string gprid,int gprdata[])
{
/*set_gpr is used to constuct the gpr command which is '#' followed by 3 bytes are formatted (MSB) as HHHHLLLL, LGGGGGGG, GGGGGGGG where H=shelf, L=slot, G=GPR ID and arg1 byte(0-4)bit mask and arg2 byte(0-4) value.In this function 2nd to 4th byte is first formed in binary and then converted to hex and finally the entire hex commandis formed.*/

	long int shlf_slt_hex;
	long int slt_gpr_hex;
	long int str1_hex;
	unsigned char cmd[GPR_ARRAY_SIZE] ={0};
	unsigned char cmdascii[GPR_ARRAY_SIZE]={0};
	int call_result =0;
	std::string data;

   	bitset<4> myshelf = bitset<4>(atoi(shelfId.c_str()));
	std::string bin_shelf = (string) (myshelf.to_string());	
	
	bitset<5> myslot = bitset<5>(slot);
	std::string bin_slot = (string) (myslot.to_string()); 	

	bitset<15> mygpr = bitset<15>(atoi(gprid.c_str()));
	std::string bin_gprid = (string) (mygpr.to_string());

	std::string slotstr = bin_slot.substr(0,4);
	std::string shlf_slt= bin_shelf+slotstr;

	stringtohex(shlf_slt,shlf_slt_hex);

	std::string str= bin_slot.substr(4,5);

	std::string gprstr = bin_gprid.substr(0,7);
	std::string slt_gpr= str+gprstr;
	
	stringtohex(slt_gpr,slt_gpr_hex);
	
	std::string str1 = bin_gprid.substr(7,15);

	stringtohex(str1,str1_hex);

	cmd[0]= 0x23;

	cmd[1] = (unsigned char)shlf_slt_hex;

	cmd[2] = (unsigned char)slt_gpr_hex;

	cmd[3] = (unsigned char)str1_hex;

	for(int i=0;i<8;i++)
	{
		cmd[i+4]= gprdata[i];
	}
	for(int j=0;j<12;j++)
	{
		cmdascii[j]=(unsigned char)cmd[j];
	}
	std::string mac_cmd(reinterpret_cast<const char *> (cmdascii),
                     sizeof (cmdascii) / sizeof (cmdascii[0]));

        FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0], _dmxc_addresses[1]);
        FIXS_CCH_IronsideUdpCommand*  set_gpr((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));
	call_result = set_gpr->executeUdp(mac_cmd, data);
	if (call_result  == 0)
        {
                data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

                if (data.compare("OK") != 0)
                {
                        call_result = IRONSIDE_FAILURE;
                }

        }

        else
        {
                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : set GPR failed",LOG_LEVEL_ERROR);
                delete set_gpr;
                set_gpr = NULL;
                return IRONSIDE_FAILURE;
        }
        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager :set GPR success",LOG_LEVEL_DEBUG);
        delete set_gpr;
        set_gpr = NULL;
        return call_result;
	
	
}

void FIXS_CCH_IronsideManager::stringtohex(std::string strval,long int &hexval)
{
	hexval=0;
	bitset<8> set(strval); 
	std::stringstream ss;
	ss<<std::hex<<set.to_ulong()<<endl;
	ss>>hexval;	
}
int FIXS_CCH_IronsideManager::reset_board(
                uint32_t board_magazine,
                int32_t board_slot)

{
	char filter[16] = {0};
	int call_result = 0;

	char board_magazine_str [16] = {0};
	int mag_phys_addr = -1;
       
	int resp = isBoardblocked(board_magazine,board_slot);

	if (resp!=0)
	{
           return resp;
	}

 
        //Here we convert Shelf Address in a.b.c.d format
        if (const int call_result = uint32_to_ip_format(board_magazine_str, board_magazine)) // ERROR: while converting
                return call_result;

        mag_phys_addr = shelf_addr_to_physical_addr(board_magazine_str);

        if (mag_phys_addr == -1)
                return CCH_Util::CCH_ERR_IP_ADDRESS_CONVERSION;

        FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0], _dmxc_addresses[1]);
        FIXS_CCH_IronsideUdpCommand*  board_gpr((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));

        //Convert shelf and slot info from integer to char
        char shelf[16] = {0};
        char slot[16] = {0};

        sprintf(shelf,"%d",mag_phys_addr);
        sprintf(slot,"%d",board_slot);

        std::string shelfId("");
        std::string resettype="";
        std::string gracefulreset="";
        
        if (get_shelfId_from_address(mag_phys_addr,shelfId) != CCH_Util::CCH_ERR_NO_ERRORS)
        {
           char tmpStr[512] = {0};
           snprintf(tmpStr, sizeof(tmpStr) - 1, "Exiting Method %s@%d, magazine %s ,(physical address %d) not found!)", __FUNCTION__, __LINE__,board_magazine_str, mag_phys_addr);
           
           if (FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

           return CCH_Util::CCH_ERR_DMX_MAGAZINE_NOT_FOUND; 
        }
	
       
/*        if(type == IMM_Util::RESET_NORMAL)*/
        {
          resettype="HARD";
          gracefulreset ="FALSE";
        }
/*        else if( type == IMM_Util::RESET_FACTORY_DEFAULT)
        {
           resettype="FACTORY_DEFAULT";
           gracefulreset ="FALSE";
        }*/

    
        //call_result = board_gpr->resetBoard(tenant,"",shelfId,slot,resettype,gracefulreset);
	snprintf(filter, 13, "RST:%02d%02dHARD",atoi(shelfId.c_str()),board_slot);
	std::string udp_data="";
        if (call_result == 0)
        {
                if (board_gpr->executeUdp(filter,udp_data) == 0)
                {
			udp_data.erase(std::remove(udp_data.begin(), udp_data.end(), '\n'), udp_data.end());
			if (udp_data.compare("OK") != 0)
			{
	                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Reset of the board with UDP failed",LOG_LEVEL_ERROR);
        	                delete board_gpr;
                	        board_gpr = NULL;
                        	return IRONSIDE_FAILURE;
			}
                }
        }

        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Reset of the board with UDP successful",LOG_LEVEL_DEBUG);
        delete board_gpr;
        board_gpr = NULL;
        return call_result;
}

int FIXS_CCH_IronsideManager::setBootType(
                uint32_t board_magazine,
                int32_t board_slot,
                IMM_Util::BootType type )
{
	int call_result = 0;
	std::string gprid="";
	//std::string gprdata ="255.255.255.255.255.255.255.255";
	int  gprdata[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	char board_magazine_str [16] = {0};
	int mag_phys_addr = -1;

	//Here we convert Shelf Address in a.b.c.d format
	if (const int call_result = uint32_to_ip_format(board_magazine_str, board_magazine)) // ERROR: while converting
		return call_result;

	mag_phys_addr = shelf_addr_to_physical_addr(board_magazine_str);

	if (mag_phys_addr == -1)
		return CCH_Util::CCH_ERR_IP_ADDRESS_CONVERSION;

	//Convert shelf and slot info from integer to char
	char shelf[16] = {0};
	char slot[16] = {0};

	sprintf(shelf,"%d",mag_phys_addr);
	sprintf(slot,"%d",board_slot);

	std::string shelfId("");
	if (get_shelfId_from_address(mag_phys_addr,shelfId) != CCH_Util::CCH_ERR_NO_ERRORS)
	{

		return CCH_Util::CCH_ERR_DMX_MAGAZINE_NOT_FOUND;
	}

        std::string boardConfiguration("");
        if (get_boardConfiguration(shelfId,board_slot,boardConfiguration) != CCH_Util::CCH_ERR_NO_ERRORS)
	{
        	return IRONSIDE_FAILURE;
	}

        if (boardConfiguration.compare("N/A") == 0)
        { 
		if(type == IMM_Util::FLASH_BOOT)
		{
			gprid ="512";
//			gprdata= "255.255.255.255.10.6.255.255";
			gprdata[4] = 0x0A;
			gprdata[5]=0x06;
		}
		else if(type == IMM_Util::NETWORK_BOOT)
		{
			gprid ="512";
		//	gprdata="255.255.255.255.2.6.255.255";
			gprdata[4]=0x02;
			gprdata[5]=0x06;
		}
		call_result=setGprData(shelfId,board_slot,gprid,gprdata);
		if(call_result!=CCH_Util::CCH_ERR_NO_ERRORS)
		{
			cout << "Ironside command for setting GprData failed";
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Setting GprData failed",LOG_LEVEL_ERROR);
			return IRONSIDE_FAILURE;
		}
	}
        else   
        {
		call_result = setboardConfiguration(shelfId,board_slot,type);
 		if(call_result!=CCH_Util::CCH_ERR_NO_ERRORS)
               	{
			if(type == IMM_Util::FLASH_BOOT)
			{
                        	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Setting BCS flashboot failed",LOG_LEVEL_ERROR);
			}
			else if(type == IMM_Util::NETWORK_BOOT){
				if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager :Networkboot BCS on A-plane failed",LOG_LEVEL_ERROR);
			}
                        return IRONSIDE_FAILURE;
		}
	}
	return call_result;
}

int FIXS_CCH_IronsideManager::setboardConfiguration(std::string shelfId, int32_t slot, IMM_Util::BootType type )
{
        FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0], _dmxc_addresses[1]);
        FIXS_CCH_IronsideUdpCommand*  board_conf((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));
	
	int call_result =0;
	char filter[16] = {0};
	char filter1[16] = {0};
	char filter2[16] = {0};
	std::string data;

	if (type == IMM_Util::FLASH_BOOT)
	{
		snprintf(filter, 13, "BCS:%x%02d0FF18",atoi(shelfId.c_str()),slot);
		snprintf(filter1, 13, "BCS:%x%02d1FF01",atoi(shelfId.c_str()),slot);
	}	
	else if (type == IMM_Util::NETWORK_BOOT)
	{
                snprintf(filter, 13, "BCS:%x%02d0FF00",atoi(shelfId.c_str()),slot);
                snprintf(filter1, 13, "BCS:%x%02d1FF01",atoi(shelfId.c_str()),slot);
	}

	if ((call_result = board_conf->executeUdp(filter, data)) == 0)
        {
                data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

                if (data.compare("OK") != 0)
                {
			if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager :setting board configuration failed",LOG_LEVEL_ERROR);
			delete board_conf;
			board_conf = NULL;
                        return IRONSIDE_FAILURE;
                }
		
        }
        if ((call_result = board_conf->executeUdp(filter1, data)) == 0)
        {
                data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());

                if (data.compare("OK") != 0)
                {
                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager :setting board configuration failed",LOG_LEVEL_ERROR);
                        delete board_conf;
                        board_conf = NULL;
                        return IRONSIDE_FAILURE;
                }

        }
       
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager :setting board configuration passed",LOG_LEVEL_DEBUG);
	delete board_conf;
	board_conf = NULL;
	return call_result;
}

int FIXS_CCH_IronsideManager::setUpgradeType(
                uint32_t board_magazine,
                int32_t board_slot,
                IMM_Util::BoardUpgradeType type )
{
	int call_result = 0;
	std::string gprid ="";
	int gprdata[8] = {0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00};
//	std::string gprdata ="0.0.255.255.0.0.0.0";

	char board_magazine_str [16] = {0};
	int mag_phys_addr = -1;

	//Here we convert Shelf Address in a.b.c.d format
	if (const int call_result = uint32_to_ip_format(board_magazine_str, board_magazine)) // ERROR: while converting
		return call_result;

	mag_phys_addr = shelf_addr_to_physical_addr(board_magazine_str);

	if (mag_phys_addr == -1)
		return CCH_Util::CCH_ERR_IP_ADDRESS_CONVERSION;

	//Convert shelf and slot info from integer to char
	char shelf[16] = {0};
	char slot[16] = {0};

	sprintf(shelf,"%d",mag_phys_addr);
	sprintf(slot,"%d",board_slot);

	std::string shelfId("");
	if (get_shelfId_from_address(mag_phys_addr,shelfId) != CCH_Util::CCH_ERR_NO_ERRORS)
	{
		char tmpStr[512] = {0};
		snprintf(tmpStr, sizeof(tmpStr) - 1, "Exiting Method %s@%d, magazine %s ,(physical address %d) not found!)", __FUNCTION__, __LINE__,board_magazine_str, mag_phys_addr);

		if (FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);

		return CCH_Util::CCH_ERR_DMX_MAGAZINE_NOT_FOUND;
	}

	if(type == IMM_Util::NORMAL_UPGRADE)
	{
		gprid="263";
//		gprdata ="0.0.255.255.0.0.0.1";
		gprdata[7] = 0x01;
		
	}
	else if( type == IMM_Util::INITIAL_INSTALLATION)
	{
		gprid="263";
//		gprdata ="0.0.255.255.0.0.1.1";
		gprdata[6] = 0x01;
		gprdata[7] = 0x01;
	}
	if(setGprData(shelfId,board_slot,gprid,gprdata)!= CCH_Util::CCH_ERR_NO_ERRORS)
	{
		cout << "Ironside command for setting GprData failed";
		if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Setting GprData failed",LOG_LEVEL_ERROR);
		return IRONSIDE_FAILURE;
	}

	return call_result;
}

int FIXS_CCH_IronsideManager::getGprData( std::string &gprDataValue,uint32_t board_magazine,
                int32_t board_slot, std::string gprId)
{
	int call_result = 0;
	char filter[16] = {0};
         char board_magazine_str [16] = {0};
        int mag_phys_addr = -1;

        //Here we convert Shelf Address in a.b.c.d format
        if (const int call_result = uint32_to_ip_format(board_magazine_str, board_magazine)) // ERROR: while converting
                return call_result;

        mag_phys_addr = shelf_addr_to_physical_addr(board_magazine_str);

        if (mag_phys_addr == -1)
                return CCH_Util::CCH_ERR_IP_ADDRESS_CONVERSION;

        FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0], _dmxc_addresses[1]);
        FIXS_CCH_IronsideUdpCommand*  board_gpr((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));

        //Convert shelf and slot info from integer to char
        char shelf[16] = {0};
        char slot[16] = {0};

        sprintf(shelf,"%d",mag_phys_addr);
        sprintf(slot,"%d",board_slot);

        std::string shelfId("");
       
        if (get_shelfId_from_address(mag_phys_addr,shelfId) != CCH_Util::CCH_ERR_NO_ERRORS)
        {
           char tmpStr[512] = {0};
           snprintf(tmpStr, sizeof(tmpStr) - 1, "Exiting Method %s@%d, magazine %s ,(physical address %d) not found!)", __FUNCTION__, __LINE__,board_magazine_str, mag_phys_addr);
           
           if (FIXS_CCH_logging)  FIXS_CCH_logging->Write(tmpStr,LOG_LEVEL_ERROR);
 
           return CCH_Util::CCH_ERR_DMX_MAGAZINE_NOT_FOUND; 
        }

	snprintf(filter, 13, "GPR:%02d%02d%d-",atoi(shelfId.c_str()),board_slot,atoi(gprId.c_str()));
	std::string udp_data="";
	call_result = board_gpr->executeUdp(filter,udp_data);
        if( call_result ==0)
	{
                udp_data.erase(std::remove(udp_data.begin(), udp_data.end(), '\n'), udp_data.end());
                if ((strcmp(udp_data.c_str(),CCH_NOTRDY)== 0) || (strcmp(udp_data.c_str(),CCH_UNSPEC) == 0))
                {
			cout << "Ironside command for getting GprData failed";
                        if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Getting GprData failed",LOG_LEVEL_ERROR);
                	delete board_gpr;
                	board_gpr = NULL;
                        return IRONSIDE_FAILURE;
                }
		gprDataValue=udp_data;
	}
	else
	{
                cout << "Ironside command for getting GprData failed";
                if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Getting GprData failed",LOG_LEVEL_ERROR);
		delete board_gpr;
                board_gpr = NULL;
                return IRONSIDE_FAILURE;

	}
	if(FIXS_CCH_logging)  FIXS_CCH_logging->Write("FIXS_CCH_IronsideManager : Getting GprData Success ",LOG_LEVEL_DEBUG);
        delete board_gpr;
        board_gpr = NULL;
	return EXECUTED; 
}

int FIXS_CCH_IronsideManager::get_boardConfiguration(std::string shelfId, int32_t slot, std::string &boardConfigurationStr)
{
        int call_result = 0;
	boardConfigurationStr ="";
        FIXS_CCH_IronsideCommandBuilder commandBuilder(_dmxc_addresses[0],_dmxc_addresses[1]);
        FIXS_CCH_IronsideUdpCommand*  boardConfiguration((FIXS_CCH_IronsideUdpCommand*) commandBuilder.make(ironsideBuilder::UDP));
	
        char filter[16] = {0};
        std::string data;
	
	snprintf(filter, 13, "BCS:%x%02d_GET_",atoi(shelfId.c_str()),slot);
	call_result = boardConfiguration->executeUdp(filter, data);
	if (call_result == 0)
        {
                data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
                if ((strcmp(data.c_str(),CCH_NOTRDY)== 0) || (strcmp(data.c_str(),CCH_UNSPEC) == 0))
                {
                	delete boardConfiguration;
                	boardConfiguration = NULL;
                        return IRONSIDE_FAILURE;
                }
		
		boardConfigurationStr=data;
	}
	else
	{
                delete boardConfiguration;
                boardConfiguration = NULL;
                return IRONSIDE_FAILURE;

	}
                delete boardConfiguration;
                boardConfiguration = NULL;
		return call_result;	
	
}

