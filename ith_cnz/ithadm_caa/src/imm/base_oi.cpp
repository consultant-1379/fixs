/*
 * fixs_ith_base_oi.cpp
 *
 *  Created on: Feb 22, 2016
 *      Author: eanform
 */

#include "imm/base_oi.h"

#include "fixs_ith_programmacros.h"
#include "fixs_ith_logger.h"

#include <ace/Future.h>

namespace imm
{

	Base_OI::Base_OI(const std::string &managedObjectClassName, const std::string &objectImplementerName) :
			ACE_Event_Handler(),
			acs_apgcc_objectimplementerinterface_V3(objectImplementerName),
			m_currentOperationId(operation::NOOP),
			m_managedObjectClassName(managedObjectClassName),
			m_managedObjectName(),
			m_objectImplementerName(objectImplementerName),
			m_immRegistered(false),
			m_reactorRegistered(false)
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "CREATED IMPLEMENTER. ECIM CLASS: <%s>, OI NAME: <%s>", m_managedObjectClassName.c_str(), m_objectImplementerName.c_str());
	}

	Base_OI::~Base_OI()
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "DESTROYED IMPLEMENTER <%s>", m_managedObjectClassName.c_str());
	}

	int Base_OI::handle_input (ACE_HANDLE /*fd*/)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "IMM Handle has been signaled for <%s>", m_managedObjectClassName.c_str());

		ACS_CC_ReturnType result = dispatch(ACS_APGCC_DISPATCH_ONE);
		if(ACS_CC_SUCCESS != result)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "<%s> dispatch failed!! result: <%d>", m_managedObjectClassName.c_str(), result);
		}
		else
		{
			FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "dispatch succeeded for <%s>", m_managedObjectClassName.c_str());
		}

		//handle_input always returns 0, otherwise the reactore removes Base_OI from its handlers
		return fixs_ith::ERR_NO_ERRORS;
	}

	int Base_OI::handle_close (ACE_HANDLE /*handle*/, ACE_Reactor_Mask /*close_mask*/)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "Ignore Handle Close for <%s>", m_managedObjectClassName.c_str());

		return fixs_ith::ERR_NO_ERRORS;
	}


	ACS_CC_ReturnType Base_OI::create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "In");

		UNUSED(oiHandle);
		UNUSED(ccbId);
		UNUSED(className);
		UNUSED(parentname);
		UNUSED(attr);

		return ACS_CC_SUCCESS;
	}

	ACS_CC_ReturnType Base_OI::deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "In");
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"**** DELETE OBJ: <%s> ****", objName);

		UNUSED(oiHandle);
		UNUSED(ccbId);
		UNUSED(objName);

		return ACS_CC_SUCCESS;
	}

	ACS_CC_ReturnType Base_OI::modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"**** MODIFY OBJ: <%s> ****", objName);

		UNUSED(oiHandle);
		UNUSED(ccbId);
		UNUSED(attrMods);

		return ACS_CC_SUCCESS;
	}

	ACS_CC_ReturnType Base_OI::complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "In");
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"**** COMPLETE CALLBACK ****");

		UNUSED(oiHandle);
		UNUSED(ccbId);

		return ACS_CC_SUCCESS;
	}

	void Base_OI::abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "In");
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"**** ABORT CALLBACK ****");

		UNUSED(oiHandle);
		UNUSED(ccbId);

		return;
	}

	void Base_OI::apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "In");
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"**** APPLY CALLBACK ****");

		UNUSED(oiHandle);
		UNUSED(ccbId);

		return;
	}

	ACS_CC_ReturnType Base_OI::updateRuntime(const char* p_objName, const char** p_attrName)
	{
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG, "In");
		FIXS_ITH_LOG(LOG_LEVEL_DEBUG,"**** UPDATE OBJ: <%s> ****", p_objName);

		UNUSED(p_objName);
		UNUSED(p_attrName);

		return ACS_CC_SUCCESS;
	}

	void Base_OI::adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList)
	{
		FIXS_ITH_LOG(LOG_LEVEL_INFO, "IMM Action requested: oiHandle == %llu, invocation == %llu, p_objName == '%s' , operationId == %llu", oiHandle, invocation, p_objName, operationId);

		UNUSED(paramList);

		//  invoke the correct action handler
		int action_result = SA_AIS_OK;

		//switch(operationId) and set action_result

		// set operation result
		ACS_CC_ReturnType imm_res = adminOperationResult(oiHandle, invocation, action_result);
		if(imm_res != ACS_CC_SUCCESS)
		{
			FIXS_ITH_LOG(LOG_LEVEL_ERROR, "Call 'ACS_APGCC_RuntimeOwner_V3::adminOperationResult(..)' failed ! call_result == %d, imm last error == %d, imm last error text == '%s'",
				imm_res, getInternalLastError(), getInternalLastErrorText());
		}

		FIXS_ITH_LOG(LOG_LEVEL_INFO, "IMM Action executed. Result == %d. ", action_result);
	}

} /* namespace imm */

