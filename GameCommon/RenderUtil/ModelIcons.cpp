
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "ModelIcons.h"

class ModelIcon
{
public:

	void		UpdateIconTexture();
	void		SetLighting();
	void		Release();

	ModelIconHandle		mhModelIcon;
	float		mfRotationAngle;
	float		mfRotationSpeed;
	ulong		mulLastUpdateTick;
	int			mhModelHandle;
	int			mhTexture;
	int			mhRenderTargetTexture;
	int			mFlags;

	ModelIcon*	mpNext;
};

int		msnModelIconsNextHandle = 0x100;
ModelIcon*		mspModelIconsList = NULL;
ModelIcon*		mspNextToUpdate = NULL;

void		ModelIcon::SetLighting()
{
ENGINE_LIGHT	xLight;
VECT		xVect = { 0.25f, 0.25f, -0.75f };

	VectNormalize( &xVect );

	// Add a light to the scene
	memset( &xLight, 0, sizeof( xLight ) );
	xLight.Type = DIRECTIONAL_LIGHT;
    xLight.Diffuse.r = 0.9f;
	xLight.Diffuse.g = 0.9f;
    xLight.Diffuse.b = 0.9f; 
	xLight.Ambient.r = 0.4f;
	xLight.Ambient.g = 0.4f;
	xLight.Ambient.b = 0.4f;
    xLight.Specular.r = 0.7f;
	xLight.Specular.g = 0.7f;
    xLight.Specular.b = 0.7f;
    xLight.Direction = xVect;

	EngineActivateLight( 0, &xLight );

	EngineEnableLighting( 1 );
}

 
void		ModelIcon::Release()
{
	EngineReleaseTexture( &mhTexture );
	EngineReleaseTexture( &mhRenderTargetTexture );
	ModelFree( mhModelHandle );
	mhModelHandle = NOTFOUND;
}

void		ModelIcon::UpdateIconTexture()
{
VECT	xCamPos = { -1.0f, -1.0f, 1.0f };
VECT	xCamDir = { 0.5f, 0.5f, -0.5f };
VECT	xCamUp = { 0.0f, 0.0f, 1.0f };
ulong	ulTick;
float	fDelta;
VECT	xPos = { 0.0f, 0.0f, 0.0f };
VECT	xRot = { 0.0f, 0.0f, 0.0f };
float	fCamDist;

	EngineDefaultState();
	
	SetLighting();
	VectNormalize( &xCamDir );
	fCamDist = ModelGetStats(mhModelHandle)->fBoundSphereRadius * 2.0f;
	VectScale( &xCamPos, &xCamPos, fCamDist );
	ulTick = SysGetTick();

	fDelta = (float)(ulTick - mulLastUpdateTick) * 0.001f;
	mulLastUpdateTick = ulTick;
	mfRotationAngle += fDelta * mfRotationSpeed * 0.4f;

	xRot.z = mfRotationAngle;

	EngineSetRenderTargetTexture( mhRenderTargetTexture, 0x00000000, TRUE );

	EngineCameraSetPos( xCamPos.x, xCamPos.y, xCamPos.z );
	EngineCameraSetDirection( xCamDir.x, xCamDir.y, xCamDir.z );
	EngineCameraSetUpVect( xCamUp.x, xCamUp.y, xCamUp.z );
	EngineCameraUpdate();

	EngineCameraSetViewAspectOverride( 1.0f );
	EngineCameraSetProjection( A45, 0.1f, 50.0f );

//	EngineSetMaterial( ENGINEMATERIAL* 
	EngineSetTexture( 0, mhTexture );
	EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
	ModelRender( mhModelHandle, &xPos, &xRot, RENDER_FLAGS_NO_STATE_CHANGE );
	
	EngineCameraSetViewAspectOverride( 0.0f );
	EngineRestoreRenderTarget();


}


//-----------------------------------------------

void		ModelIconsInit( void )
{

}

ModelIconHandle		ModelIconCreate(  const char* szModel, const char* szTexture,  eModelIconFlags flags, int nLoadFromArchive )
{
int		nNewHandle = msnModelIconsNextHandle++;
ModelIcon*		pNewModelIcon;

	pNewModelIcon = new ModelIcon;
	pNewModelIcon->mhModelIcon = nNewHandle;
	pNewModelIcon->mpNext = mspModelIconsList;
	mspModelIconsList = pNewModelIcon;

	pNewModelIcon->mFlags = flags;
	pNewModelIcon->mfRotationAngle = FRand( 0.0f, A360 );
	pNewModelIcon->mfRotationSpeed = 1.0f;
	if ( nLoadFromArchive != NOTFOUND )
	{
		pNewModelIcon->mhModelHandle = ModelLoadFromArchive( szModel, 0, 1.0f, nLoadFromArchive );
	}
	else
	{
		pNewModelIcon->mhModelHandle = ModelLoad( szModel, 0, 1.0f );
	}
	
	if ( nLoadFromArchive != NOTFOUND )
	{
		pNewModelIcon->mhTexture = EngineLoadTextureFromArchive( szTexture, 0, nLoadFromArchive, NULL );
	}
	else
	{
		pNewModelIcon->mhTexture = EngineLoadTexture( szTexture, 0, NULL );
	}
	pNewModelIcon->mhRenderTargetTexture = EngineCreateRenderTargetTexture( 256, 256, 1 );
	pNewModelIcon->mulLastUpdateTick = SysGetTick();

	return( nNewHandle );
}

void		ModelIconsUpdate( void )
{
	if ( mspNextToUpdate == NULL )
	{
		mspNextToUpdate = mspModelIconsList;
	}

	if ( mspNextToUpdate )
	{
		mspNextToUpdate->UpdateIconTexture();
		mspNextToUpdate = mspNextToUpdate->mpNext;
	}

}

int		ModelIconGetIconTexture( ModelIconHandle handle )
{
ModelIcon*		pModelIcons = mspModelIconsList;
	
	while( pModelIcons )
	{
		if ( pModelIcons->mhModelIcon == handle )
		{
			return( pModelIcons->mhRenderTargetTexture );
		}
		pModelIcons = pModelIcons->mpNext;
	}

	return( NULL );
}


void		ModelIconsRelease( ModelIconHandle handle )
{
ModelIcon*		pModelIcons = mspModelIconsList;
ModelIcon*		pLast = NULL;

	while( pModelIcons )
	{
		if ( pModelIcons->mhModelIcon == handle )
		{
			if ( pLast == NULL )
			{
				mspModelIconsList = pModelIcons->mpNext;
			}
			else
			{
				pLast->mpNext = pModelIcons->mpNext;
			}
			pModelIcons->Release();

			if ( mspNextToUpdate == pModelIcons ) mspNextToUpdate = NULL;
			delete pModelIcons;
			return;
		}
		pLast = pModelIcons;
		pModelIcons = pModelIcons->mpNext;
	}

}

void		ModelIconsFreeAll( void )
{
ModelIcon*		pModelIcons = mspModelIconsList;
ModelIcon*		pNext;

	while( pModelIcons )
	{
		pNext = pModelIcons->mpNext;
		pModelIcons->Release();
		delete pModelIcons;
		pModelIcons = pNext;
	}
	mspModelIconsList = NULL;
	mspNextToUpdate = NULL;
}

void		ModelIconsShutdown( void )
{
	ModelIconsFreeAll();
}
