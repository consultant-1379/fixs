//	********************************************************
//
//	 COPYRIGHT Ericsson 2015
//	All rights reserved.
//
//	The Copyright to the computer program(s) herein
//	is the property of Ericsson 2015.
//	The program(s) may be used and/or copied only with
//	the written permission from Ericsson 2015 or in
//	accordance with the terms and conditions stipulated in
//	the agreement/contract under which the program(s) have
//	been supplied.
//
//	********************************************************

#ifndef ITHADM_CAA_INC_FIXS_ITH_BNC_H_
#define ITHADM_CAA_INC_FIXS_ITH_BNC_H_

#include <string>
#include <set>

#include "fixs_ith_programconstants.h"
#include "fixs_ith_programmacros.h"
#include "fixs_ith_switchboardinfo.h"
#include "operation/operation.h"
#include "imm/imm.h"


namespace engine
{
	namespace control
	{
		static const uint16_t ENABLED_INTERFACES_INDEXES[] = {CONTROL_PLANE_BP_INDEXES, CONTROL_PLANE_SWITCH_PORTS_INDEXES, CONTROL_PLANE_TRANSPORT_PORTS_INDEXES, CONTROL_PLANE_INTER_SUBRACK_PORT_INDEX}; //BP1-BP24, S_LOCAL, S_REMOTE, TRANSPORT1, 10G1
		static const std::set<uint16_t> ENABLED_INTERFACES_INDEXES_SET(ENABLED_INTERFACES_INDEXES, ENABLED_INTERFACES_INDEXES + FIXS_ITH_ARRAY_SIZE(ENABLED_INTERFACES_INDEXES));

		static const uint16_t MONITORED_INTERFACES_INDEXES[] = { CONTROL_PLANE_INTER_SUBRACK_PORT_INDEX }; //10G1
		static const std::set<uint16_t> MONITORED_INTERFACES_INDEXES_SET(MONITORED_INTERFACES_INDEXES, MONITORED_INTERFACES_INDEXES + FIXS_ITH_ARRAY_SIZE(MONITORED_INTERFACES_INDEXES));

		static const uint16_t BP_INTERFACES_INDEXES[] = { CONTROL_PLANE_BP_INDEXES };
		static const std::set<uint16_t> BP_INTERFACES_INDEXES_SET(BP_INTERFACES_INDEXES, BP_INTERFACES_INDEXES + FIXS_ITH_ARRAY_SIZE(BP_INTERFACES_INDEXES));

		static const uint16_t TRANSPORTPORTS_INTERFACES_INDEXES[] = { CONTROL_PLANE_TRANSPORT1_PORT_INDEX, CONTROL_PLANE_TRANSPORT2_PORT_INDEX };
		static const std::set<uint16_t> TRANSPORTPORTS_INTERFACES_INDEXES_SET(TRANSPORTPORTS_INTERFACES_INDEXES, TRANSPORTPORTS_INTERFACES_INDEXES + FIXS_ITH_ARRAY_SIZE(TRANSPORTPORTS_INTERFACES_INDEXES));
	}

	namespace transport
	{
		static const uint16_t ENABLED_INTERFACES_INDEXES[] = {TRANSPORT_PLANE_BP_INDEXES, TRANSPORT_PLANE_INTER_SUBRACK_PORT_INDEX, TRANSPORT_PLANE_AGG_MEMBER_INDEXES, TRANSPORT_PLANE_CONTROL_PORTS_INDEXES, TRANSPORT_PLANE_AGGREGATOR_INDEX}; //BP1-BP24, A1, B1_1-4, B2_1-4, CONTROL1, LAG
		static const std::set<uint16_t> ENABLED_INTERFACES_INDEXES_SET(ENABLED_INTERFACES_INDEXES, ENABLED_INTERFACES_INDEXES + FIXS_ITH_ARRAY_SIZE(ENABLED_INTERFACES_INDEXES));

		static const uint16_t EARLY_ENABLED_IF_INDEXES[] = {TRANSPORT_PLANE_AGG_MEMBER_INDEXES, TRANSPORT_PLANE_AGGREGATOR_INDEX}; //B1_1-4, B2_1-4,LAG
		static const std::set<uint16_t> EARLY_ENABLED_IF_INDEXES_SET(EARLY_ENABLED_IF_INDEXES, EARLY_ENABLED_IF_INDEXES + FIXS_ITH_ARRAY_SIZE(EARLY_ENABLED_IF_INDEXES));

		static const uint16_t MONITORED_INTERFACES_INDEXES[] = { TRANSPORT_PLANE_INTER_SUBRACK_PORT_INDEX, TRANSPORT_PLANE_AGG_MEMBER_INDEXES }; //A1, B1_1-4, B2_1-4
		static const std::set<uint16_t> MONITORED_INTERFACES_INDEXES_SET(MONITORED_INTERFACES_INDEXES, MONITORED_INTERFACES_INDEXES + FIXS_ITH_ARRAY_SIZE(MONITORED_INTERFACES_INDEXES));

		static const uint16_t BP_INTERFACES_INDEXES[] = { TRANSPORT_PLANE_BP_INDEXES };
		static const std::set<uint16_t> BP_INTERFACES_INDEXES_SET(BP_INTERFACES_INDEXES, BP_INTERFACES_INDEXES + FIXS_ITH_ARRAY_SIZE(BP_INTERFACES_INDEXES));

		static const uint16_t CONTROLPORTS_INTERFACES_INDEXES[] = { TRANSPORT_PLANE_CONTROL1_PORT_INDEX, TRANSPORT_PLANE_CONTROL2_PORT_INDEX };
		static const std::set<uint16_t> CONTROLPORTS_INTERFACES_INDEXES_SET(CONTROLPORTS_INTERFACES_INDEXES, CONTROLPORTS_INTERFACES_INDEXES + FIXS_ITH_ARRAY_SIZE(CONTROLPORTS_INTERFACES_INDEXES));

		// Added for CMX
                static const uint16_t ENABLED_INTERFACES_INDEXES_CMX[] = {TRANSPORT_PLANE_BP_INDEXES_CMX, TRANSPORT_PLANE_FRONT_PORT_INDEXES_CMX, TRANSPORT_PLANE_AGGREGATOR_INDEX};
				
		static const std::set<uint16_t> ENABLED_INTERFACES_INDEXES_SET_CMX(ENABLED_INTERFACES_INDEXES_CMX, ENABLED_INTERFACES_INDEXES_CMX + FIXS_ITH_ARRAY_SIZE(ENABLED_INTERFACES_INDEXES_CMX));
				
		static const uint16_t EARLY_ENABLED_IF_INDEXES_CMX[] = {TRANSPORT_PLANE_AGG_MEMBER_INDEXES_CMX, TRANSPORT_PLANE_AGGREGATOR_INDEX};
				
                static const std::set<uint16_t> EARLY_ENABLED_IF_INDEXES_SET_CMX(EARLY_ENABLED_IF_INDEXES_CMX, EARLY_ENABLED_IF_INDEXES_CMX + FIXS_ITH_ARRAY_SIZE(EARLY_ENABLED_IF_INDEXES_CMX));

                static const uint16_t BP_INTERFACES_INDEXES_CMX[] = { TRANSPORT_PLANE_BP_INDEXES_CMX };

                static const std::set<uint16_t> BP_INTERFACES_INDEXES_SET_CMX(BP_INTERFACES_INDEXES_CMX, BP_INTERFACES_INDEXES_CMX + FIXS_ITH_ARRAY_SIZE(BP_INTERFACES_INDEXES_CMX));


	}

	typedef struct
	{
		std::string name;
		fixs_ith::magazine_identifier_t magazine_id;
		fixs_ith::switchboard_plane_t plane;
		fixs_ith::board_slotposition_t slot;
		uint16_t vid;
		std::set<uint16_t> taggedPortsIndexes;
		std::set<uint16_t> untaggedPortsIndexes;

	} static_vlan_t;

	namespace defaultAggregator
	{
		static const std::string NAME = "LA1";
		static const fixs_ith::switchboard_plane_t PLANE = fixs_ith::TRANSPORT_PLANE;
		static const uint16_t SNMP_INDEX = TRANSPORT_PLANE_AGGREGATOR_INDEX;
		static const uint16_t AGG_MEMBER_INDEXES[] = {TRANSPORT_PLANE_AGG_MEMBER_INDEXES}; //B1_1-4 and B2_1-4 front ports
		static const fixs_ith::magazine_identifier_t MAGAZINE_ID = fixs_ith::APZ_MAGAZINE;
		static const std::set<uint16_t> AGG_MEMBER_INDEXES_SET(AGG_MEMBER_INDEXES, AGG_MEMBER_INDEXES + FIXS_ITH_ARRAY_SIZE(AGG_MEMBER_INDEXES));

	};

        namespace defaultAggregatorCMX
        {
                static const std::string NAME = "LA1";
                static const fixs_ith::switchboard_plane_t PLANE = fixs_ith::TRANSPORT_PLANE;
                static const uint16_t SNMP_INDEX = TRANSPORT_PLANE_AGGREGATOR_INDEX;
                static const uint16_t AGG_MEMBER_INDEXES[] = {TRANSPORT_PLANE_AGG_MEMBER_INDEXES_CMX}; // E3 and E4 front ports
                static const fixs_ith::magazine_identifier_t MAGAZINE_ID = fixs_ith::APZ_MAGAZINE;
                static const std::set<uint16_t> AGG_MEMBER_INDEXES_SET(AGG_MEMBER_INDEXES, AGG_MEMBER_INDEXES + FIXS_ITH_ARRAY_SIZE(AGG_MEMBER_INDEXES));

        };


	namespace defaultVlan
	{
		namespace control
		{
			namespace left
			{
				static const std::string NAME = "APZ-A";
				static const fixs_ith::board_slotposition_t SLOT = fixs_ith::BOARD_SMX_LEFT;
			}

			namespace right
			{
				static const std::string NAME = "APZ-B";
				static const fixs_ith::board_slotposition_t SLOT = fixs_ith::BOARD_SMX_RIGHT;
			}

			static const fixs_ith::switchboard_plane_t PLANE = fixs_ith::CONTROL_PLANE;
			static const fixs_ith::magazine_identifier_t MAGAZINE_ID = fixs_ith::ALL_MAGAZINE;
			static const uint16_t VID = 1;
			static const uint16_t UNTAGGED_PORTS_INDEXES[] = {CONTROL_PLANE_BP_INDEXES, CONTROL_PLANE_SWITCH_PORTS_INDEXES, CONTROL_PLANE_INTER_SUBRACK_PORT_INDEX}; //BP1-BP24, S_LOCAL, S_REMOTE, 10G1
			//static const uint16_t TAGGED_PORTS_INDEXES[] = {}; //empty
			static const std::set<uint16_t> UNTAGGED_PORTS_INDEXES_SET(UNTAGGED_PORTS_INDEXES, UNTAGGED_PORTS_INDEXES + FIXS_ITH_ARRAY_SIZE(UNTAGGED_PORTS_INDEXES));
			static const std::set<uint16_t> TAGGED_PORTS_INDEXES_SET;
		}

		namespace transport
		{
			namespace left
			{
				static const std::string NAME = "Default";
				static const fixs_ith::board_slotposition_t SLOT = fixs_ith::BOARD_SMX_LEFT;
			}

			namespace right
			{
				static const std::string NAME = "Default";
				static const fixs_ith::board_slotposition_t SLOT = fixs_ith::BOARD_SMX_RIGHT;
			}

			static const fixs_ith::switchboard_plane_t PLANE = fixs_ith::TRANSPORT_PLANE;
			static const fixs_ith::magazine_identifier_t MAGAZINE_ID = fixs_ith::ALL_MAGAZINE;
			static const uint16_t VID = 1;
			static const std::set<uint16_t> UNTAGGED_PORTS_INDEXES_SET;
			static const std::set<uint16_t> TAGGED_PORTS_INDEXES_SET;
		}
	}

	namespace tipcVlan
	{
		namespace control
		{
			static const std::string NAME = "APG-TIPC";
			static const fixs_ith::switchboard_plane_t PLANE = fixs_ith::CONTROL_PLANE;
			static const fixs_ith::magazine_identifier_t MAGAZINE_ID = fixs_ith::APZ_MAGAZINE;
			//static const uint16_t UNTAGGED_PORTS_INDEXES[] = {}; //empty
			static const uint16_t TAGGED_PORTS_INDEXES[] = {CONTROL_PLANE_BP_INDEXES}; //BP1-BP24
			static const std::set<uint16_t> UNTAGGED_PORTS_INDEXES_SET;
			static const std::set<uint16_t> TAGGED_PORTS_INDEXES_SET(TAGGED_PORTS_INDEXES, TAGGED_PORTS_INDEXES + FIXS_ITH_ARRAY_SIZE(TAGGED_PORTS_INDEXES));
		}
	}

	namespace drbdVlan
	{
		namespace transport
		{
			static const std::string NAME = "APG-DRBD";
			static const fixs_ith::switchboard_plane_t PLANE = fixs_ith::TRANSPORT_PLANE;
			static const fixs_ith::magazine_identifier_t MAGAZINE_ID = fixs_ith::APZ_MAGAZINE;
			//static const uint16_t UNTAGGED_PORTS_INDEXES[] = {}; //empty
			static const uint16_t TAGGED_PORTS_INDEXES[] = {TRANSPORT_PLANE_BP_INDEXES, defaultAggregator::SNMP_INDEX}; //BP1-BP24, LAG
			static const std::set<uint16_t> UNTAGGED_PORTS_INDEXES_SET;
			static const std::set<uint16_t> TAGGED_PORTS_INDEXES_SET(TAGGED_PORTS_INDEXES, TAGGED_PORTS_INDEXES + FIXS_ITH_ARRAY_SIZE(TAGGED_PORTS_INDEXES));
			static const uint16_t TAGGED_PORTS_INDEXES_CMX[] = {TRANSPORT_PLANE_BP_INDEXES_CMX, defaultAggregatorCMX::SNMP_INDEX};
			static const std::set<uint16_t> TAGGED_PORTS_INDEXES_SET_CMX(TAGGED_PORTS_INDEXES_CMX, TAGGED_PORTS_INDEXES_CMX + FIXS_ITH_ARRAY_SIZE(TAGGED_PORTS_INDEXES_CMX));

		}
	}
	
	namespace updVlan
	{
		namespace transport
		{
                        //GEP2_GEP5 to GEP7 Dynamic UPD VLAN feature. 
			namespace left
                        {
                                static const std::string NAME = "APZ-UPD-A";
                                static const uint16_t VID_LEFT = 1301;
                                static const fixs_ith::board_slotposition_t SLOT = fixs_ith::BOARD_SMX_LEFT;
                        }

                        namespace right
                        {
                                static const std::string NAME = "APZ-UPD-B";
                                static const uint16_t VID_RIGHT = 1302;
                                static const fixs_ith::board_slotposition_t SLOT = fixs_ith::BOARD_SMX_RIGHT;
                        }


                        namespace cmxleft
                        {
                                static const std::string NAME = "APZ-UPD-A";
                                static const uint16_t VID_LEFT = 1301;
                                static const fixs_ith::board_slotposition_t SLOT = fixs_ith::BOARD_CMX_LEFT;
                        }

                        namespace cmxright
                        {
                                static const std::string NAME = "APZ-UPD-B";
                                static const uint16_t VID_RIGHT = 1302;
                                static const fixs_ith::board_slotposition_t SLOT = fixs_ith::BOARD_CMX_RIGHT;
                        }


                        //GEP2_GEP5 to GEP7 Dynamic UPD VLAN END 
	
			static const fixs_ith::switchboard_plane_t PLANE = fixs_ith::TRANSPORT_PLANE;
			static const fixs_ith::magazine_identifier_t MAGAZINE_ID = fixs_ith::APZ_MAGAZINE;
			//static const uint16_t UNTAGGED_PORTS_INDEXES[] = {}; //empty
			static const uint16_t TAGGED_PORTS_INDEXES[] = {TRANSPORT_PLANE_BP_INDEXES, defaultAggregator::SNMP_INDEX}; //BP1-BP24, LAG
			static const std::set<uint16_t> UNTAGGED_PORTS_INDEXES_SET;
			static const std::set<uint16_t> TAGGED_PORTS_INDEXES_SET(TAGGED_PORTS_INDEXES, TAGGED_PORTS_INDEXES + FIXS_ITH_ARRAY_SIZE(TAGGED_PORTS_INDEXES));
			static const uint16_t TAGGED_PORTS_INDEXES_CMX[] = {TRANSPORT_PLANE_BP_INDEXES_CMX, defaultAggregatorCMX::SNMP_INDEX};
			static const std::set<uint16_t> TAGGED_PORTS_INDEXES_SET_CMX(TAGGED_PORTS_INDEXES_CMX, TAGGED_PORTS_INDEXES_CMX + FIXS_ITH_ARRAY_SIZE(TAGGED_PORTS_INDEXES_CMX));
		}
	}

	namespace apgOamVlan
	{
		namespace transport
		{
			static const std::string NAME = "OM_Inner";
			static const fixs_ith::switchboard_plane_t PLANE = fixs_ith::TRANSPORT_PLANE;
			static const fixs_ith::magazine_identifier_t MAGAZINE_ID = fixs_ith::APZ_MAGAZINE;
			//static const uint16_t UNTAGGED_PORTS_INDEXES[] = {}; //empty
			static const uint16_t TAGGED_PORTS_INDEXES[] = {TRANSPORT_PLANE_BP_INDEXES, defaultAggregator::SNMP_INDEX, TRANSPORT_PLANE_CONTROL_PORTS_INDEXES}; //BP1-BP24, LAG, CONTROL
			static const std::set<uint16_t> UNTAGGED_PORTS_INDEXES_SET;
			static const std::set<uint16_t> TAGGED_PORTS_INDEXES_SET(TAGGED_PORTS_INDEXES, TAGGED_PORTS_INDEXES + FIXS_ITH_ARRAY_SIZE(TAGGED_PORTS_INDEXES));
			static const uint16_t TAGGED_PORTS_INDEXES_CMX[] = {TRANSPORT_PLANE_BP_INDEXES_CMX, defaultAggregatorCMX::SNMP_INDEX};
			static const std::set<uint16_t> TAGGED_PORTS_INDEXES_SET_CMX(TAGGED_PORTS_INDEXES_CMX, TAGGED_PORTS_INDEXES_CMX + FIXS_ITH_ARRAY_SIZE(TAGGED_PORTS_INDEXES_CMX));
		}
	}
	//modification not completed, remove this comment later
        namespace evoEtIngress
        {
                namespace transport
                {
                        static const std::string NAME = "EVOET-INGRESS";
			static const uint16_t VID = 180;
                        static const fixs_ith::switchboard_plane_t PLANE = fixs_ith::TRANSPORT_PLANE;
                        static const fixs_ith::magazine_identifier_t MAGAZINE_ID = fixs_ith::ALL_MAGAZINE;
                        //static const uint16_t UNTAGGED_PORTS_INDEXES[] = {}; //empty
                        static const uint16_t TAGGED_PORTS_INDEXES[] = {TRANSPORT_PLANE_BP_INDEXES_CMX, defaultAggregatorCMX::SNMP_INDEX, TRANSPORT_PLANE_FRONT_PORT_EVOET_CMX}; //BP1-BP24, E1-E8 , LAG
                        static const std::set<uint16_t> UNTAGGED_PORTS_INDEXES_SET;
                        static const std::set<uint16_t> TAGGED_PORTS_INDEXES_SET(TAGGED_PORTS_INDEXES, TAGGED_PORTS_INDEXES + FIXS_ITH_ARRAY_SIZE(TAGGED_PORTS_INDEXES));
                }
        }


	namespace defaultRouter
	{
		static const std::string NAME = "0";
		static const fixs_ith::magazine_identifier_t MAGAZINE_ID = fixs_ith::APZ_MAGAZINE;
	}

}


#endif /* ITHADM_CAA_INC_FIXS_ITH_BNC_H_ */
