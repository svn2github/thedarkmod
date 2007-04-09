/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 866 $
 * $Date: 2007-03-23 22:25:02 +0100 (Fr, 23 M�r 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/
#ifndef SR_RESPONSE__H
#define SR_RESPONSE__H

#include "StimResponse.h"

#include "ResponseEffect.h"
class CStim;

class CResponse : public CStimResponse {
friend CStimResponseCollection;

public:
	/**
	* This method is called when the response should
	* make its script callback. It is virtual
	* so that the container can reach overriden
	* versions from a CStimResponse base pointer.
	*
	* @sourceEntity: This is the entity carrying the stim
	* @stim: This is the stim to retrieve stim properties like magnitude, etc.
	*		 This is an optional argument, pass NULL to fire responses without
	*		 a "real" stim (e.g. frobbing)
	*/
	virtual void TriggerResponse(idEntity *sourceEntity, CStim* stim = NULL);

	/**
	 * Set the response script action.
	 */
	void SetResponseAction(idStr const &ActionScriptName);

	/**
	* Adds a response effect and returns the pointer to the new Effect object.
	*
	* @effectEntityDef: The entity definition where the target script is stored.
	*					The effect entity "effect_script" is treated specially.
	*
	* @effectPostfix:	The string that gets passed to the effect script (e.g. "1_2")
	*
	* @args:	The entity's spawnargs needed to query the script argument for the
	*			aforementioned special case of "effect_script".
	*/
	CResponseEffect* addResponseEffect(const idStr& effectEntityDef, 
									   const idStr& effectPostfix,
									   const idDict *args);

protected:
	CResponse(idEntity *Owner, int Type);
	virtual ~CResponse(void);

protected:
	/**
	 * Once this stimuls is finished, another one can be fired. So you
	 * can chain multiple stimulis one after the other. If NULL this is
	 * a single event.
	 */
	CResponse			*m_FollowUp;

	/**
	 * Scriptfunction that is to be executed when this response 
	 * is triggered.
	 */
	idStr				m_ScriptFunction;

	/**
	 * How much damage must be applied for this response?
	 */
	float				m_MinDamage;

	/**
	 * No more than this.
	 */
	float				m_MaxDamage;

	/**
	* The list of ResponseEffects
	*/
	idList<CResponseEffect*> m_ResponseEffects;
};

#endif /* SR_RESPONSE__H */
