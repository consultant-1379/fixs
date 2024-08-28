/*
 * FIXS_CCH_BladePercistanceValue.h
 *
 *  Created on: Jun 4, 2012
 *      Author: eanform
 */

#ifndef FIXS_CCH_BLADEPERCISTANCEVALUE_H_
#define FIXS_CCH_BLADEPERCISTANCEVALUE_H_


#include <iostream>

#include "acs_apgcc_objectimplementerinterface_V3.h"
#include "ACS_CC_Types.h"
#include "acs_apgcc_adminoperationtypes.h"
#include "acs_apgcc_omhandler.h"
#include "acs_apgcc_adminoperation.h"
#include "ACS_APGCC_Util.H"
#include "ACS_TRA_Logging.h"

#include "FIXS_CCH_UpgradeManager.h"
#include "FIXS_CCH_SNMPManager.h"
#include "FIXS_CCH_DiskHandler.h"
#include "FIXS_CCH_IronsideManager.h"

class FIXS_CCH_BladePercistanceValue: public acs_apgcc_objectimplementerinterface_V3 {
public:

	FIXS_CCH_BladePercistanceValue();

	virtual ~FIXS_CCH_BladePercistanceValue();

	/*the callback*/

	/** @brief create method
	 *
	 *	This method will be called as a callback when an Object is created as instance of a Class CpVolume
	 *	All input parameters are input provided by IMMSV Application and have to be used by the implementer to perform
	 *	proper actions.
	 *
	 *  @param oiHandle: the object implementer handle. This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param ccbId: the ID for the Configuration Change Bundle in which the creation of the Object is contained.
	 *	This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param className: the name of the class. When an object is created as instance of this class this method is
	 *	called if the application has registered as class implementer. This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param parentname: the name of the parent object for the object now creating.
	 *	This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param attr: a pointer to a null terminated array of ACS_APGCC_AttrValues element pointers each one containing
	 *	the info about the attributes belonging to the now creating class.
	 *	This is an Input Parameter provided by IMMSV Application.
	 *
	 *  @return ACS_CC_ReturnType On success ACS_CC_SUCCESS on Failure ACS_CC_FAILURE.
	 */
	virtual ACS_CC_ReturnType create(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *className, const char* parentname, ACS_APGCC_AttrValues **attr);

	/** @brief deleted method
	 *
	 *	This method will be called as a callback when deleting an CpVolume Object
	 *	Object Implementer. All input parameters are input provided by IMMSV Application and have to be used by
	 *	the implementer to perform proper actions.
	 *
	 *  @param oiHandle: the object implementer handle. This is an Input Parametr provided by IMMSV Application.
	 *
	 *	@param ccbId: the ID for the Configuration Change Bundle in which the deletion of the Object is contained.
	 *	This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param objName: the Distinguished name of the object that has to be deleted.
	 *	This is an Input Parameter provided by IMMSV Application.
	 *
	 *  @return ACS_CC_ReturnType On success ACS_CC_SUCCESS on Failure ACS_CC_FAILURE.
	 *
	 *	@remarks Remarks
	 */
	virtual ACS_CC_ReturnType deleted(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName);

	/** @brief modify method
	 *
	 *	This method will be called as a callback when modifying an CpVolume Object
	 *	All input parameters are input provided by IMMSV Application and have to be used by the implementer to perform
	 *	proper actions.
	 *
	 *  @param oiHandle: the object implementer handle. This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param ccbId: the ID for the Configuration Change Bundle in which the modify of the Object is contained.
	 *	This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param objName: the Distinguished name of the object that has to be modified.
	 *	This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param attrMods: a NULL terminated array of pointers to ACS_APGCC_AttrModification elements containing
	 *	the information about the modify to perform. This is an Input Parametr provided by IMMSV Application.
	 *
	 *  @return ACS_CC_ReturnType On success ACS_CC_SUCCESS on Failure ACS_CC_FAILURE.
	 *
	 *	@remarks Remarks
	 */
	virtual ACS_CC_ReturnType modify(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId, const char *objName, ACS_APGCC_AttrModification **attrMods);

	/** @brief complete method
	 *
	 *	This method will be called as a callback when a Configuration Change Bundle is complete and can be applied
	 *	regarding  an CpVolume Object
	 *
	 *  @param oiHandle: the object implementer handle. This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param ccbId: the ID for the Configuration Change Bundle in which the modify of the Object is contained.
	 *	This is an Input Parameter provided by IMMSV Application.
	 *
	 *  @return ACS_CC_ReturnType On success ACS_CC_SUCCESS on Failure ACS_CC_FAILURE.
	 *
	 *	@remarks Remarks
	 */
	virtual ACS_CC_ReturnType complete(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

	/** @brief abort method
	 *
	 *	This method will be called as a callback when a Configuration Change Bundle, regarding  an CpVolume Object,
	 *	has aborted. This method is called only if at least one complete method failed.
	 *
	 *  @param oiHandle: the object implementer handle. This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param ccbId: the ID for the Configuration Change Bundle containing actions on Objects for which the Application
	 *	registered as Object Implementer. This is an Input Parameter provided by IMMSV Application.
	 *
	 *  @return ACS_CC_ReturnType On success ACS_CC_SUCCESS on Failure ACS_CC_FAILURE.
	 *
	 *	@remarks Remarks
	 */
	virtual void abort(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

	/** @brief apply method
	 *
	 *	This method will be called as a callback when a Configuration Change Bundle, regarding  an CpVolume Object, is complete and can be applied.
	 *	This method is called only if all the complete method have been successfully executed.
	 *
	 *  @param oiHandle: the object implementer handle. This is an Input Parameter provided by IMMSV Application.
	 *
	 *	@param ccbId: the ID for the Configuration Change Bundle containing actions on Objects for which the Application
	 *	registered as Object Implementer. This is an Input Parameter provided by IMMSV Application.
	 *
	 *  @return ACS_CC_ReturnType On success ACS_CC_SUCCESS on Failure ACS_CC_FAILURE.
	 *
	 *	@remarks Remarks
	 */
	virtual void apply(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_CcbId ccbId);

	/** @brief updateRuntime method
	 *
	 * 	This method will be called as a callback when modifying a runtime not-cached attribute of a configuration Object
	 * 	for which the Application has registered as Object Implementer.
	 * 	All input parameters are input provided by IMMSV Application and have to be used by the implementer
	 * 	to perform proper actions.
	 *
	 *  @param p_objName: the Distinguished name of the object that has to be modified.
	 *  				  This is an Input Parameter provided by IMMSV Application.
	 *
	 *  @param p_attrName: the name of attribute that has to be modified.
	 *  				   This is an Input Parameter provided by IMMSV Application.
	 *
	 *  @return ACS_CC_ReturnType On success ACS_CC_SUCCESS on Failure ACS_CC_FAILURE.
	 *
	 *	@remarks Remarks
	 */
	virtual ACS_CC_ReturnType updateRuntime(const char* p_objName, const char** p_attrName);

	/**
	 * @brief adminOperationCallback method
	 * adminOperationCallback method: This method will be called as a callback to manage an administrative operation invoked, on the
	 * composite CP File object.
	 *
	 * @param  p_objName:	the distinguished name of the object for which the administrative operation
	 * has to be managed.
	 *
	 * @param  oiHandle : input parameter,ACS_APGCC_OiHandle this value has to be used for returning the callback
	 *  result to IMM.
	 *
	 * @param invocation: input parameter,the invocation id used to match the invocation of the callback with the invocation
	 * of result function
	 *
	 * @param  p_objName: input parameter,the name of the object
	 *
	 * @param  operationId: input parameter, the administrative operation identifier
	 *
	 * @param paramList: a null terminated array of pointers to operation params elements. each element of the list
	 * is a pointer toACS_APGCC_AdminOperationParamType element holding the params provided to the Administretive operation..
	 *
	 * @return ACS_CC_ReturnType. On success ACS_CC_SUCCESS on Failure ACS_CC_FAILURE
	 */
	virtual void adminOperationCallback(ACS_APGCC_OiHandle oiHandle,ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList);



	/** @brief getClassName method
	 *
	 * 	This method return the name of the IMM class.
	 *
	 */
	const char* getIMMClassName() const {return m_ImmClassName.c_str();};
	void setAdminOperationError(int errorCode,ACS_APGCC_OiHandle oiHandle,ACS_APGCC_InvocationType invocation);


private:

	std::string m_ImmClassName;

	ACS_TRA_Logging *FIXS_CCH_logging;

	int resetBoard (std::string dn_value, IMM_Util::BoardResetType type);

	int resetBoardSCXB (unsigned long umagazine, unsigned short uslot, IMM_Util::BoardResetType type);

	int resetBoardSMXB (unsigned long umagazine, unsigned short uslot, IMM_Util::BoardResetType type);

	int resetBoardIPTB (unsigned long umagazine, unsigned short uslot, IMM_Util::BoardResetType type);
	
	int resetBoardIPLB (unsigned long umagazine, unsigned short uslot, IMM_Util::BoardResetType type);

	int resetBoardCMXB (unsigned long umagazine, unsigned short uslot, IMM_Util::BoardResetType type);

	bool discoverShelfManager(unsigned long umagazine, std::string &shelfMgrIpA, std::string &shelfMgrIpB);

	bool isBoardAccessible(std::string & shelfMgrIpA, std::string & shelfMgrIpB, unsigned short & uslot);

	bool isBoardTurnedOn(std::string & shelfMgrIpA, std::string & shelfMgrIpB, unsigned short & uslot);

	int boardBlock (std::string dn_value, bool force);

	int blockIPLB(unsigned long umagazine, unsigned short uslot);

	int blockIPTB(unsigned long umagazine, unsigned short uslot);

	int boardDeblock (std::string dn_value);

	int deblockIPLB(unsigned long umagazine, unsigned short uslot);

	int deblockIPTB(unsigned long umagazine, unsigned short uslot);

};


#endif /* FIXS_CCH_BLADEPERCISTANCEVALUE_H_ */
