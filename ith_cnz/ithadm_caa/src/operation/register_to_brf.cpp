/*
 * register_to_brf.cpp
 *
 *  Created on: Sep 7, 2016
 *      Author: xassore
 */

#include "operation/register_to_brf.h"
#include "fixs_ith_programconstants.h"

#include "fixs_ith_logger.h"
#include "fixs_ith_workingset.h"

namespace operation {

const int IMM_OBJECT_ALREADY_EXIST = -14;

	register_to_brf::register_to_brf()
	 : OperationBase(REGISTER_TO_BRF)
	{

	}

	int register_to_brf::call()
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, " ->>> 'Register to BRF as Backup participant' operation requested");

		int call_result = fixs_ith::ERR_NO_ERRORS;

		// step 1: create an object of  BrfPersistantDataOwner class
		if((call_result = create_brf_participant()) == fixs_ith::ERR_NO_ERRORS)
		{
			// step 2: register as Object Implementer on PersistantDataOwner MO.
			call_result =  fixs_ith::workingSet_t::instance()->registerAsBrfPartecipant();
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BRF Object Implementers registration result:<%d>", call_result );

		}
		if (call_result != 0)
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'BRF Object Implementer Registration' failed: cannot register as BRF Backup participant");

		else
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Successful registered as BRF Backup participant ");


		// set result to caller
		m_operationResult.setErrorCode(call_result);
		setResultToCaller();

		return fixs_ith::ERR_NO_ERRORS;
}


int register_to_brf::create_brf_participant()
{
	int result = fixs_ith::ERR_NO_ERRORS;

	OmHandler om_handler;

	if(om_handler.Init() != ACS_CC_SUCCESS)
	{
		// ERROR initializing imm om handler
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Internal OM IMM handler init failure: imm last error:<%d>, imm last error text:<%s>",
						om_handler.getInternalLastError(), om_handler.getInternalLastErrorText());
		return fixs_ith::ERR_OM_HANDLER_INIT_FAILURE;
	}

	//Create attributes list
	vector < ACS_CC_ValuesDefinitionType > AttrList;
	ACS_CC_ValuesDefinitionType attributeRDN;
	ACS_CC_ValuesDefinitionType attributeVersion;
	ACS_CC_ValuesDefinitionType attributeBackupType;

	/*Fill the rdn Attribute */
	char rdnValue[IMM_RDN_PATH_SIZE_MAX] = {0};
	strncpy(rdnValue, imm::brfPersistentDataOwner::BRF_OBJECT_NAME.c_str(),IMM_RDN_PATH_SIZE_MAX-1) ;

	char attrdn[IMM_ATTR_NAME_MAX_SIZE] = {0};
	strncpy(attrdn, imm::brfPersistentDataOwner::BRF_RDN.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	/*Fill the rdn Attribute */
	attributeRDN.attrName = attrdn;
	attributeRDN.attrType = ATTR_STRINGT;
	attributeRDN.attrValuesNum = 1;
	void* attrRdnValue[1] = { reinterpret_cast<void*>(rdnValue) };
	attributeRDN.attrValues = attrRdnValue;


	char attrVersion[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(attrVersion, imm::brfPersistentDataOwner::BRF_ATTR_VERSION.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attributeVersion.attrName = attrVersion;
	attributeVersion.attrType = ATTR_STRINGT;
	attributeVersion.attrValuesNum = 1;
	char* strValue = const_cast<char*>("1.0");
	void* valueStr[1] = { reinterpret_cast<void*>(strValue) };
	attributeVersion.attrValues = valueStr;

	char attrBackupType[IMM_ATTR_NAME_MAX_SIZE]= {0};
	strncpy(attrBackupType, imm::brfPersistentDataOwner::BRF_ATTR_BACKPTYPE.c_str(),IMM_ATTR_NAME_MAX_SIZE-1) ;
	attributeBackupType.attrName = attrBackupType;
	attributeBackupType.attrType = ATTR_INT32T;
	attributeBackupType.attrValuesNum = 1;
	int intAttributeBackupType = 1;
	void* valueInt[1] = { reinterpret_cast<void*>(&intAttributeBackupType) };
	attributeBackupType.attrValues = valueInt;

	AttrList.push_back(attributeRDN);
	AttrList.push_back(attributeVersion);
	AttrList.push_back(attributeBackupType);


	FIXS_ITH_LOG(LOG_LEVEL_INFO, "Creating <%s> object", rdnValue);

	char className[IMM_CLASS_NAME_MAX_SIZE]= {0};
	strncpy(className, imm::brfPersistentDataOwner::BRF_CLASS_NAME.c_str(),IMM_CLASS_NAME_MAX_SIZE-1) ;

	if( om_handler.createObject(className, imm::brf_mom_dn::BRF_MOC_DN, AttrList) == ACS_CC_FAILURE )
	{
		int immErrorCode = om_handler.getInternalLastError();

		if( IMM_OBJECT_ALREADY_EXIST == immErrorCode )
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "BRF object <%s> ALREADY EXIST", rdnValue);
		}
		else
		{
			result = fixs_ith::ERR_IMM_CREATE_OBJ;
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Create of BRF object:<%s>  FAILED, error_code:<%d> errorText:<%s>",
					rdnValue, immErrorCode, om_handler.getInternalLastErrorText());
		}
	}

	if(om_handler.Finalize() != ACS_CC_SUCCESS)
	{
		// ERROR finalizing imm internal om handler
		FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Internal OM IMM handler finalize failure: imm last error:<%d>, imm last error text:<%s>",
				om_handler.getInternalLastError(), om_handler.getInternalLastErrorText());
	}

	return result;
}


} /* namespace operation */



