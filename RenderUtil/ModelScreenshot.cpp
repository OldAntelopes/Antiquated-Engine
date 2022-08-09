
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"


VECT	mxModelScreenShotCamPos;
VECT	mxModelScreenShotCamDir;
VECT	mxModelScreenShotCamUp;


void		ModelScreenShotSetCamera( const VECT* pxCamPos, const VECT* pxCamDir, const VECT* pxCamUp )
{
	mxModelScreenShotCamPos = *pxCamPos;
	mxModelScreenShotCamDir = *pxCamDir;
	mxModelScreenShotCamUp = *pxCamUp;

}

int		ModelScreenShotCreate( int hModel, int hTexture, int nTextureSizeX, int nTextureSizeY, int nFlags )
{
int		hRenderTexture;

	hRenderTexture = EngineCreateRenderTargetTexture( nTextureSizeX, nTextureSizeY, 3 );

	EngineSetRenderTargetTexture( hRenderTexture, 0x00ffffff, TRUE );

	EngineCameraSetPos( mxModelScreenShotCamPos.x, mxModelScreenShotCamPos.y, mxModelScreenShotCamPos.z );
	EngineCameraSetDirection( mxModelScreenShotCamDir.x, mxModelScreenShotCamDir.y, mxModelScreenShotCamDir.z );
	EngineCameraSetUpVect( mxModelScreenShotCamUp.x, mxModelScreenShotCamUp.y, mxModelScreenShotCamUp.z );
	EngineCameraUpdate();

	EngineCameraSetViewAspectOverride( 1.0f );
	EngineCameraSetProjection( A45, 0.1f, 50.0f );
	// todo - set projection

	EngineSetTexture( 0, hTexture );
	ModelRender( hModel, NULL, NULL, RENDER_FLAGS_NO_STATE_CHANGE );
	
	EngineCameraSetViewAspectOverride( 0.0f );
	EngineRestoreRenderTarget();

	return( hRenderTexture );
}


