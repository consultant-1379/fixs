/*
 * fixs_ith_base_oi.h
 *
 *  Created on: Feb 22, 2016
 *      Author: eanform
 */

#ifndef ITHADM_CAA_INC_IMM_BASE_OI_H_
#define ITHADM_CAA_INC_IMM_BASE_OI_H_

#include "acs_apgcc_objectimplementerinterface_V3.h"
#include "operation/operation.h"
#include "imm/imm.h"

#include <ace/Event_Handler.h>
#include <boost/noncopyable.hpp>

namespace imm
{
	/**
	 * @class Base_OI
	 *
	 * @brief
	 * Base class for all Object Implementers.
	 *
	 * Base_OI implements operations that are common to all Object Implementers.
	 *
	 * @sa OIHandler
	 */
	class Base_OI : public ACE_Event_Handler, public acs_apgcc_objectimplementerinterface_V3, private boost::noncopyable
	{
 	public:
		///  Constructor.
		Base_OI(const std::string &managedObjectClassName, const std::string &m_objectImplementerName);

		/// Destructor is virtual to enable proper cleanup.
		virtual ~Base_OI();

		//-----------------------------------------------------------------------------------------------------
		// Implement ACE_Event_Handler methods: callbacks from the Main Reactor
		//-----------------------------------------------------------------------------------------------------

		/// Called when input events occur (e.g., connection or data).
		virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);

		/// Called when a handle_*() method returns -1 or when the
		/// remove_handler() method is called on an ACE_Reactor.  The
		/// @a close_mask indicates which event has triggered the
		/// handle_close() method callback on a particular @a handle.
		virtual int handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask);

		/** @brief getIMMClassName method
		 *
		 * 	This method return the name of the IMM class.
		 *
		 *  @return const char pointer of the IMM class name
		 *
		 *  @remarks Remarks
		 */

		//-----------------------------------------------------------------------------------------------------
		// Implement acs_apgcc_objectimplementerinterface_V3 methods: callbacks from IMM
		//-----------------------------------------------------------------------------------------------------
		/** @brief create method
		 *
		 *	This method will be called as a callback when an Object is created as instance of the Managed Object Class
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
		 *	This method will be called as a callback when deleting a Managed Object
		 *	Object Implementer. All input parameters are input provided by IMMSV Application and have to be used by
		 *	the implementer to perform proper actions.
		 *
		 *  @param oiHandle: the object implementer handle. This is an Input Parameter provided by IMMSV Application.
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
		 *	This method will be called as a callback when modifying a Managed Object
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
		 *	regarding  a Managed Object
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
		 *	This method will be called as a callback when a Configuration Change Bundle, regarding a Managed Object,
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
		 *	This method will be called as a callback when a Configuration Change Bundle, regarding a Managed Object, is complete and can be applied.
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
		 * Managed Object.
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
		virtual void adminOperationCallback(ACS_APGCC_OiHandle oiHandle, ACS_APGCC_InvocationType invocation, const char* p_objName, ACS_APGCC_AdminOperationIdType operationId,  ACS_APGCC_AdminOperationParamType** paramList);


		//-----------------------------------------------------------------------------------------------------
		// Base_OI methods
		//-----------------------------------------------------------------------------------------------------
		/** @brief getIMMClassName method
		 *
		 * 	This method return the name of the Managed Object Class.
		 *
		 *  @return const char pointer of the MOC name
		 *
		 *  @remarks Remarks
		 */
		const char* getIMMClassName() const {return m_managedObjectClassName.c_str();};

		//-----------------------------------------------------------------------------------------------------
		// Base_OI methods
		//-----------------------------------------------------------------------------------------------------
		/** @brief getIMMObjectName method
		 *
		 * 	This method return the name of the Managed Object .
		 *
		 *  @return const char pointer of the MO instance
		 *
		 *  @remarks Remarks
		 */
		const char* getIMMObjectName() const {return m_managedObjectName.c_str();};

		//-----------------------------------------------------------------------------------------------------
		// Base_OI methods
		//-----------------------------------------------------------------------------------------------------
		/** @brief getIMMObjectName method
		 *
		 * 	This method return the name of the Managed Object .
		 *
		 *  @return const char pointer of the MO instance
		 *
		 *  @remarks Remarks
		 */
		void setIMMObjectName(const std::string &objectname) {m_managedObjectName.assign(objectname);};

		/** @brief getObjectImplementerName method
		 *
		 * 	This method return the name of the IMM object implementer.
		 *
		 *  @return const char pointer of the IMM object implementer
		 *
		 *  @remarks Remarks
		 */
		const char* getObjectImplementerName() const {return m_objectImplementerName.c_str();};

		inline bool isImmRegistered() const { return m_immRegistered; }
		inline void setImmRegistration() { m_immRegistered = true; }
		inline void unsetImmRegistration() { m_immRegistered = false; }

		inline bool isReatorRegistered() const { return m_reactorRegistered; }
		inline void setReactorRegistration() { m_reactorRegistered = true; }
		inline void unsetReactorRegistration() { m_reactorRegistered = false; }


		struct Operation
		{

			bool exclusive;

			Operation() : exclusive(false) {};
			const bool isExclusive() const { return exclusive; };

		};

 	protected:

		operation::identifier_t  m_currentOperationId;

 	private:
		std::string m_managedObjectClassName;
		std::string m_managedObjectName;
		std::string m_objectImplementerName;

		bool m_immRegistered;
		bool m_reactorRegistered;


	};

} /* namespace imm */



#endif /* ITHADM_CAA_INC_IMM_BASE_OI_H_ */
