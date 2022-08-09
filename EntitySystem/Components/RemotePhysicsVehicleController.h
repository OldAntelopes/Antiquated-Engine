#ifndef REMOTE_PHYSICS_VEHICLE_CONTROLLER_COMPONENT_H
#define	REMOTE_PHYSICS_VEHICLE_CONTROLLER_COMPONENT_H


#include "PhysicsVehicleController.h"

class PhysicsVehicle;

enum
{
	REMOTEVEHICLE_CONTROLFLAGS_STEERLEFT = 0x1,
	REMOTEVEHICLE_CONTROLFLAGS_STEERRIGHT = 0x2,
	REMOTEVEHICLE_CONTROLFLAGS_ACCELERATE = 0x4,
	REMOTEVEHICLE_CONTROLFLAGS_BRAKE = 0x8,
};

enum
{
	REMOTEUPDATE_ELEMENT_2DPOS = 0x1,
	REMOTEUPDATE_ELEMENT_3DPOS = 0x2,
	REMOTEUPDATE_ELEMENT_ORIENTATIONSHORT = 0x4,
	REMOTEUPDATE_ELEMENT_ORIENTATIONFULL = 0x8,
	REMOTEUPDATE_ELEMENT_LINEAR_VEL2D = 0x10,
	REMOTEUPDATE_ELEMENT_LINEAR_VEL3D = 0x20,
	REMOTEUPDATE_ELEMENT_ANGULAR_VEL = 0x40,

	REMOTEUPDATE_FULL = REMOTEUPDATE_ELEMENT_3DPOS | REMOTEUPDATE_ELEMENT_ORIENTATIONFULL | REMOTEUPDATE_ELEMENT_LINEAR_VEL3D | REMOTEUPDATE_ELEMENT_ANGULAR_VEL,
};

typedef struct
{
	BYTE		bControlFlags;
	BYTE		bPacketElements;
	BYTE		bPad1;
	BYTE		bPad2;
	ulong		ulTimestamp;

} REMOTE_UPDATE_HEADER;

typedef struct
{
	float		fX;
	float		fY;
} REMOTE_UPDATE_POS2D;

typedef struct
{
	float		fX;
	float		fY;
	float		fZ;
} REMOTE_UPDATE_POS3D;

typedef struct
{
	short		wQuat1;
	short		wQuat2;
	short		wQuat3;
	short		wQuat4;
} REMOTE_UPDATE_ORIENTATIONSHORT;

typedef struct
{
	float		fQuat1;
	float		fQuat2;
	float		fQuat3;
	float		fQuat4;
} REMOTE_UPDATE_ORIENTATIONFULL;

typedef struct
{
	float		fX;
	float		fY;
} REMOTE_UPDATE_LINEARVEL2D;

typedef struct
{
	float		fX;
	float		fY;
	float		fZ;
} REMOTE_UPDATE_LINEARVEL3D;

typedef struct
{
	float		fX;
	float		fY;
	float		fZ;
} REMOTE_UPDATE_ANGULARVEL;

//------------------------------------ 
typedef struct
{
	ushort		uwControlFlags;
	BYTE		bPacketElements;
	BYTE		bPad1;
	ulong		ulTimestamp;

	float		fQuat1;
	float		fQuat2;
	float		fQuat3;
	float		fQuat4;
	
	float		fPosX;
	float		fPosY;
	float		fPosZ;

	float		fLinearVelX;
	float		fLinearVelY;
	float		fLinearVelZ;

	float		fAngularVelX;
	float		fAngularVelY;
	float		fAngularVelZ;

} REMOTEPHYSICSVEHICLE_POSITION_UPDATE;


class RemotePhysicsVehicleControllerComponent : public PhysicsVehicleControllerComponent
{
public:
	RemotePhysicsVehicleControllerComponent();

	virtual const char*			GetComponentType( void ) { return( "RemotePhysicsVehicleController" ); }

	virtual void		OnUpdateComponent( float fDelta );
	virtual void		OnRenderComponent( void );

	virtual void		OnEntityInitialised( void );

	void		OnRemotePositionUpdate( const REMOTEPHYSICSVEHICLE_POSITION_UPDATE* pxUpdate );

	virtual BOOL		IsLocalVehicle( void ) { return( FALSE ); }

private:
	void		BlendToRemotePosition( float fDelta );

	PhysicsVehicle*		mpPhysicsPredictionVehicle;

	VECT				mxLastReceivedPos;
	ulong				mulLastReceivedPosTimestamp;
	ENGINEQUATERNION	mxLastReceivedOrientation;
	ulong				mulLastReceivedOrientationTimestamp;
	VECT				mxLastReceivedLinearVel;
	ulong				mulLastReceivedLinearVelTimestamp;
	VECT				mxLastReceivedAngularVel;
	ulong				mulLastReceivedAngularVelTimestamp;

	float				mfPosBlendPhase;
};




#endif