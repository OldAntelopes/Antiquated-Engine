

#include <StandardDef.h>
#include <Rendering.h>
#include <Interface.h>
#include <Engine.h>

#ifdef USING_OPENGL
// TODO
#else
#include "DirectX/EngineCameraDX.h"
#endif

#include "EngineCamera.h"

#include "CollisionMaths/collmathsdefines.h"
#include "CollisionMaths/Plane.h"

ENGINEMATRIX			mEngineViewMatrix;

VECT				mxEngineCamPos = { 0.0f, 0.0f, 0.0f };
VECT				mxEngineCamVect = { 0.0f, 1.0f, 0.0f };
VECT				mxEngineCamUpVect = { 0.0f,0.0f,1.0f };

VECT				mxEngineCamStorePos = { 0.0f, 0.0f, 0.0f };
VECT				mxEngineCamStoreVect = { 0.0f, 1.0f, 0.0f };
VECT				mxEngineCamStoreUpVect = { 0.0f,0.0f,1.0f };
float	mfEngineCamStoreFOV;
float	mfEngineCamStoreNearClip;
float	mfEngineCamStoreFarClip;

VECT*	EngineCameraGetUpVect( void )
{
	return( &mxEngineCamUpVect );
}

/***************************************************************************
 * Function    : EngineCameraGetDirection
 * Params      :
 * Description : 
 ***************************************************************************/
VECT* EngineCameraGetDirection( void )
{
	return( &mxEngineCamVect );
}

/***************************************************************************
 * Function    : EngineCameraSetDirection
 * Params      :
 * Description : 
 ***************************************************************************/
void EngineCameraSetDirection( float fX, float fY, float fZ )
{
	mxEngineCamVect.x = fX;
	mxEngineCamVect.y = fY;
	mxEngineCamVect.z = fZ;
	VectNormalize( &mxEngineCamVect );
}

void EngineCameraSetUpVect( float fX, float fY, float fZ )
{
	mxEngineCamUpVect.x = fX;
	mxEngineCamUpVect.y = fY;
	mxEngineCamUpVect.z = fZ;
}

/***************************************************************************
 * Function    : EngineCameraGetPos
 ***************************************************************************/
VECT*	EngineCameraGetPos( void )
{
	return( &mxEngineCamPos );
}


/***************************************************************************
 * Function    : EngineCameraSetPos
 ***************************************************************************/
void	EngineCameraSetPos( float fX, float fY, float fZ )
{
	mxEngineCamPos.x = fX;
	mxEngineCamPos.y = fY;
	mxEngineCamPos.z = fZ;
}


void		EngineCameraStoreCurrent( )
{
	mxEngineCamStorePos = mxEngineCamPos;
	mxEngineCamStoreUpVect = mxEngineCamUpVect;
	mxEngineCamStoreVect = mxEngineCamVect;
	EngineCameraGetCurrentProjectionSettings( &mfEngineCamStoreFOV, &mfEngineCamStoreNearClip, &mfEngineCamStoreFarClip );
}

void		EngineCameraRestore( )
{
	mxEngineCamPos = mxEngineCamStorePos;
	mxEngineCamUpVect = mxEngineCamStoreUpVect;
	mxEngineCamVect = mxEngineCamStoreVect;
	EngineCameraSetProjection( mfEngineCamStoreFOV, mfEngineCamStoreNearClip, mfEngineCamStoreFarClip );
	EngineCameraUpdate();
}

void		EngineCameraGetMatrix( ENGINEMATRIX* pMat )
{
	*pMat = mEngineViewMatrix;
}

void	EngineCameraSetMatrix( ENGINEMATRIX* pMat ) 
{
	mEngineViewMatrix = *pMat;
}



void		EngineCameraInit( void )
{
	mxEngineCamPos.x = mxEngineCamPos.y = mxEngineCamPos.z = 0.0f;
	mxEngineCamVect.x = mxEngineCamVect.y = mxEngineCamVect.z = 0.0f;
	mxEngineCamVect.y = -1.0f;

#ifdef ENGINEDX
	EngineCameraInitDX();
#endif

}
