/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision$
 * $Date$
 * $Author$
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:33  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __PHYSICS_RIGIDBODY_H__
#define __PHYSICS_RIGIDBODY_H__

/*
===================================================================================

	Rigid body physics

	Employs an impulse based dynamic simulation which is not very accurate but
	relatively fast and still reliable due to the continuous collision detection.

===================================================================================
*/

typedef struct rididBodyIState_s {
	idVec3					position;					// position of trace model
	idMat3					orientation;				// orientation of trace model
	idVec3					linearMomentum;				// translational momentum relative to center of mass
	idVec3					angularMomentum;			// rotational momentum relative to center of mass
} rigidBodyIState_t;

typedef struct rigidBodyPState_s {
	int						atRest;						// set when simulation is suspended
	float					lastTimeStep;				// length of last time step
	idVec3					localOrigin;				// origin relative to master
	idMat3					localAxis;					// axis relative to master
	idVec6					pushVelocity;				// push velocity
	idVec3					externalForce;				// external force relative to center of mass
	idVec3					externalTorque;				// external torque relative to center of mass
	rigidBodyIState_t		i;							// state used for integration
} rigidBodyPState_t;

class idPhysics_RigidBody : public idPhysics_Base {

public:

	CLASS_PROTOTYPE( idPhysics_RigidBody );

							idPhysics_RigidBody( void );
							~idPhysics_RigidBody( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

							// initialisation
	void					SetFriction( const float linear, const float angular, const float contact );
	void					SetBouncyness( const float b );
							// same as above but drop to the floor first
	void					DropToFloor( void );
							// no contact determination and contact friction
	void					NoContact( void );
							// enable/disable activation by impact
	void					EnableImpact( void );
	void					DisableImpact( void );

public:	// common physics interface
	void					SetClipModel( idClipModel *model, float density, int id = 0, bool freeOld = true );
	idClipModel *			GetClipModel( int id = 0 ) const;
	int						GetNumClipModels( void ) const;

	void					SetMass( float mass, int id = -1 );
	float					GetMass( int id = -1 ) const;

	void					SetContents( int contents, int id = -1 );
	int						GetContents( int id = -1 ) const;

	const idBounds &		GetBounds( int id = -1 ) const;
	const idBounds &		GetAbsBounds( int id = -1 ) const;

	bool					Evaluate( int timeStepMSec, int endTimeMSec );
	void					UpdateTime( int endTimeMSec );
	int						GetTime( void ) const;

	void					GetImpactInfo( const int id, const idVec3 &point, impactInfo_t *info ) const;
	void					ApplyImpulse( const int id, const idVec3 &point, const idVec3 &impulse );
	void					AddForce( const int id, const idVec3 &point, const idVec3 &force );
	void					Activate( void );
	void					PutToRest( void );
	bool					IsAtRest( void ) const;
	int						GetRestStartTime( void ) const;
	bool					IsPushable( void ) const;

	void					SaveState( void );
	void					RestoreState( void );

	void					SetOrigin( const idVec3 &newOrigin, int id = -1 );
	void					SetAxis( const idMat3 &newAxis, int id = -1 );

	void					Translate( const idVec3 &translation, int id = -1 );
	void					Rotate( const idRotation &rotation, int id = -1 );

	const idVec3 &			GetOrigin( int id = 0 ) const;
	const idMat3 &			GetAxis( int id = 0 ) const;

	void					SetLinearVelocity( const idVec3 &newLinearVelocity, int id = 0 );
	void					SetAngularVelocity( const idVec3 &newAngularVelocity, int id = 0 );

	const idVec3 &			GetLinearVelocity( int id = 0 ) const;
	const idVec3 &			GetAngularVelocity( int id = 0 ) const;

	void					ClipTranslation( trace_t &results, const idVec3 &translation, const idClipModel *model ) const;
	void					ClipRotation( trace_t &results, const idRotation &rotation, const idClipModel *model ) const;
	int						ClipContents( const idClipModel *model ) const;

	void					DisableClip( void );
	void					EnableClip( void );

	void					UnlinkClip( void );
	void					LinkClip( void );

	bool					EvaluateContacts( void );

	void					SetPushed( int deltaTime );
	const idVec3 &			GetPushedLinearVelocity( const int id = 0 ) const;
	const idVec3 &			GetPushedAngularVelocity( const int id = 0 ) const;

	void					SetMaster( idEntity *master, const bool orientated );

	void					WriteToSnapshot( idBitMsgDelta &msg ) const;
	void					ReadFromSnapshot( const idBitMsgDelta &msg );

private:
	// state of the rigid body
	rigidBodyPState_t		current;
	rigidBodyPState_t		saved;

	// rigid body properties
	float					linearFriction;				// translational friction
	float					angularFriction;			// rotational friction
	float					contactFriction;			// friction with contact surfaces
	float					bouncyness;					// bouncyness
	idClipModel *			clipModel;					// clip model used for collision detection

	// derived properties
	float					mass;						// mass of body
	float					inverseMass;				// 1 / mass
	idVec3					centerOfMass;				// center of mass of trace model
	idMat3					inertiaTensor;				// mass distribution
	idMat3					inverseInertiaTensor;		// inverse inertia tensor

	idODE *					integrator;					// integrator
	bool					dropToFloor;				// true if dropping to the floor and putting to rest
	bool					testSolid;					// true if testing for solid when dropping to the floor
	bool					noImpact;					// if true do not activate when another object collides
	bool					noContact;					// if true do not determine contacts and no contact friction

	// master
	bool					hasMaster;
	bool					isOrientated;

private:
	friend void				RigidBodyDerivatives( const float t, const void *clientData, const float *state, float *derivatives );
	void					Integrate( const float deltaTime, rigidBodyPState_t &next );
	bool					CheckForCollisions( const float deltaTime, rigidBodyPState_t &next, trace_t &collision );
	bool					CollisionImpulse( const trace_t &collision, idVec3 &impulse );
	void					ContactFriction( float deltaTime );
	void					DropToFloorAndRest( void );
	bool					TestIfAtRest( void ) const;
	void					Rest( void );
	void					DebugDraw( void );
};

#endif /* !__PHYSICS_RIGIDBODY_H__ */
