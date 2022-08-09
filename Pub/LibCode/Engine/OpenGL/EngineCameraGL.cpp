
#include <windows.h>			// For OpenGL
#include <gl/gl.h>
#include <gl/glu.h>

#include <StandardDef.h>
#include <Engine.h>

#include "../EngineCamera.h"

#include "EngineCameraGL.h"

CULL_RESULT	EngineIsBoundingSphereInView( const VECT* pxOrigin, float fRadius )
{
	// todo
	return( OUTSIDE );
//	return( Frustum::Get().ContainsSphere( pxOrigin, fRadius ) );
}
 

void		EngineCameraSetViewAspectOverride( float fAspect )
{
	/// todo mfAspectOverride = fAspect;
}

void		EngineGetScreenCoordForWorldCoord( const VECT* pPos, int* pX, int* pY )
{
	// TODO

}

/***************************************************************************
 * Function    : EngineCameraUpdate
 * Params      :
 * Description : 
 ***************************************************************************/
void EngineCameraUpdate( void )
{
   glMatrixMode(GL_MODELVIEW);

   glLoadIdentity();
   // TODO -
		// Apply current camera pos to the view transform
	gluLookAt(mxEngineCamPos.x,mxEngineCamPos.y,mxEngineCamPos.z, /* look from camera XYZ */
               mxEngineCamPos.x + mxEngineCamVect.x, mxEngineCamPos.y + mxEngineCamVect.y, mxEngineCamPos.z + mxEngineCamVect.z, /* look at the origin */
               mxEngineCamUpVect.x, mxEngineCamUpVect.y, mxEngineCamUpVect.z); 

}



void		EngineCameraSetProjection( float fFOV, float fNearClip, float fFarClip )
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective (50.0f, 1.0f/*(float)width/(float)height*/, fNearClip, fFarClip);

}