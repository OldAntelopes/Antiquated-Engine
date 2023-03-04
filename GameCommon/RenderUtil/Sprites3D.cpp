
#include "StandardDef.h"
#include "Engine.h"

#include "MultiVertexBuffers.h"
#include "Sprites3D.h"

#define NUM_SPRITE3D_VERTEX_BUFFERS		2
#define SPRITE3D_VERTEX_BUFFER_SIZE		1024

class Sprite
{
public:
	Sprite()
	{
		mfRot = 0.0f;
	}

	void		Render( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags );
	void		RenderRot( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags );

	VECT		mxPos;
	float		mfRot;
	float		mfScale;
	float		mfScaleZ;
	int			mnFrameNum;
	ulong		mulCol;
	Sprite*		mpNext;
};

class SpriteGroup
{
public:
	SpriteGroup()
	{
		mpSpriteList = NULL;
		mpNext = NULL;
		mhGroupNum = NOTFOUND;
	}

	void		Render( void );

	SPRITE_GROUP		mhGroupNum;
	int					mhTexture;
	float				mfGridScale;
	eSpriteGroupRenderFlags	mRenderFlags;
	Sprite*				mpSpriteList;
	SpriteGroup*		mpNext;
};

SpriteGroup*		mspSpriteGroups = NULL;

SPRITE_GROUP		msnNextSpriteGroupNum = 9000;


MultiVertexBuffers		mxSprites3dBuffers;

VECT		maxCamFacingSpriteOffsets[6];
VECT		maxFlatSpriteOffsets[6];
VECT		maxXAxisSpriteOffsets[6];


void	Sprite::RenderRot( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags )
{
float		fUBase,	fVBase;
float		fUWidth, fVHeight;
ulong		ulCol = mulCol;
VECT		xPos = mxPos;
VECT		xOffset;
int			nNumPerRow = (int)( 1.0f / fGridScale );
VECT		xRotAxis;
VECT*		pxSpriteOffsets = maxCamFacingSpriteOffsets;
VECT*		pxRotateAxis = EngineCameraGetDirection();

	if ( nRenderFlags & kSpriteRender_Orientation_Flat )
	{
		pxSpriteOffsets = maxFlatSpriteOffsets;
		xRotAxis.x = 0.0f;
		xRotAxis.y = 0.0f;
		xRotAxis.z = 1.0f;
		pxRotateAxis = &xRotAxis;
	}
	else if ( nRenderFlags & kSpriteRender_Orientation_XAxis )
	{
		pxSpriteOffsets = maxXAxisSpriteOffsets;
		xRotAxis.x = 0.0f;
		xRotAxis.y = 1.0f;
		xRotAxis.z = 0.0f;
		pxRotateAxis = &xRotAxis;
	}

	fUBase = (mnFrameNum % nNumPerRow) * fGridScale;
	fVBase = (mnFrameNum / nNumPerRow) * fGridScale;
	fUWidth = fGridScale;
	fVHeight = fGridScale;

	// Tri 1 - vert 1
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[0].x * mfScale;
	xOffset.y = pxSpriteOffsets[0].y * mfScale;
	xOffset.z = pxSpriteOffsets[0].z * mfScaleZ;	
	VectRotateAboutAxis( &xOffset, &xOffset, pxRotateAxis, mfRot );
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 1 - vert 2
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[1].x * mfScale;
	xOffset.y = pxSpriteOffsets[1].y * mfScale;
	xOffset.z = pxSpriteOffsets[1].z * mfScaleZ;	
	VectRotateAboutAxis( &xOffset, &xOffset, pxRotateAxis, mfRot );
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 1 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[2].x * mfScale;
	xOffset.y = pxSpriteOffsets[2].y * mfScale;
	xOffset.z = pxSpriteOffsets[2].z * mfScaleZ;	
	VectRotateAboutAxis( &xOffset, &xOffset, pxRotateAxis, mfRot );
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();
	
	//-------------------------------------------------

	// Tri 2 - vert 1
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[3].x * mfScale;
	xOffset.y = pxSpriteOffsets[3].y * mfScale;
	xOffset.z = pxSpriteOffsets[3].z * mfScaleZ;	
	VectRotateAboutAxis( &xOffset, &xOffset, pxRotateAxis, mfRot );
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 2 - vert 2
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[4].x * mfScale;
	xOffset.y = pxSpriteOffsets[4].y * mfScale;
	xOffset.z = pxSpriteOffsets[4].z * mfScaleZ;	
	VectRotateAboutAxis( &xOffset, &xOffset, pxRotateAxis, mfRot );
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	// Tri 2 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[5].x * mfScale;
	xOffset.y = pxSpriteOffsets[5].y * mfScale;
	xOffset.z = pxSpriteOffsets[5].z * mfScaleZ;	
	VectRotateAboutAxis( &xOffset, &xOffset, pxRotateAxis, mfRot );
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	pxDrawBuffer->FlushWhenFull( 6, TRUE );
}


void	Sprite::Render( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags )
{
float		fUBase,	fVBase;
float		fUWidth, fVHeight;
ulong		ulCol = mulCol;
VECT		xPos = mxPos;
VECT		xOffset;
int			nNumPerRow = (int)( 1.0f / fGridScale );
VECT*		pxSpriteOffsets = maxCamFacingSpriteOffsets;

	if ( nRenderFlags & kSpriteRender_Orientation_Flat )
	{
		pxSpriteOffsets = maxFlatSpriteOffsets;
	}
	else if ( nRenderFlags & kSpriteRender_Orientation_XAxis )
	{
		pxSpriteOffsets = maxXAxisSpriteOffsets;
	}

	fUBase = (mnFrameNum % nNumPerRow) * fGridScale;
	fVBase = (mnFrameNum / nNumPerRow) * fGridScale;
	fUWidth = fGridScale;
	fVHeight = fGridScale;

	// Tri 1 - vert 1
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[0].x * mfScale;
	xOffset.y = pxSpriteOffsets[0].y * mfScale;
	xOffset.z = pxSpriteOffsets[0].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 1 - vert 2
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[1].x * mfScale;
	xOffset.y = pxSpriteOffsets[1].y * mfScale;
	xOffset.z = pxSpriteOffsets[1].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 1 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[2].x * mfScale;
	xOffset.y = pxSpriteOffsets[2].y * mfScale;
	xOffset.z = pxSpriteOffsets[2].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();
	
	//-------------------------------------------------

	// Tri 2 - vert 1
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[3].x * mfScale;
	xOffset.y = pxSpriteOffsets[3].y * mfScale;
	xOffset.z = pxSpriteOffsets[3].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 2 - vert 2
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[4].x * mfScale;
	xOffset.y = pxSpriteOffsets[4].y * mfScale;
	xOffset.z = pxSpriteOffsets[4].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	// Tri 2 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	xOffset.x = pxSpriteOffsets[5].x * mfScale;
	xOffset.y = pxSpriteOffsets[5].y * mfScale;
	xOffset.z = pxSpriteOffsets[5].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, pxDrawBuffer->mpxPosStream, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	pxDrawBuffer->FlushWhenFull( 6, TRUE );
}

//-----------------------------------------------------------------------------

void	SpriteGroup::Render( void )
{
Sprite*		pSprites = mpSpriteList;
Sprite*		pNext;

	EngineSetTexture( 0, mhTexture );
	if ( mRenderFlags & kSpriteRender_Additive )
	{
		EngineSetBlendMode( BLEND_MODE_SRCALPHA_ADDITIVE );
	}
	else if ( mRenderFlags & kSpriteRender_ColourBlend )
	{
		EngineSetBlendMode( BLEND_MODE_COLOUR_BLEND );
		EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
	}
	else
	{
		EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
	}
	
	mxSprites3dBuffers.Lock();
	
	while( pSprites )
	{
		pNext = pSprites->mpNext;
		if ( mRenderFlags & kSpriteRender_Rotated )
		{
			pSprites->RenderRot( &mxSprites3dBuffers, mfGridScale, mRenderFlags );
		}
		else
		{
			pSprites->Render( &mxSprites3dBuffers, mfGridScale, mRenderFlags );
		}
		delete pSprites;
		pSprites = pNext;
	}
	
	mxSprites3dBuffers.FlushWhenFull( 0, FALSE );

	mpSpriteList = NULL;
}


//-----------------------------------------------------------------------------
void Sprites3DInitialiseGraphicsDeviceResources( void )
{
	mxSprites3dBuffers.Init( NUM_SPRITE3D_VERTEX_BUFFERS, SPRITE3D_VERTEX_BUFFER_SIZE, "Sprites3d" );
}


void Sprites3DReleaseGraphicsDeviceResources( void )
{
	mxSprites3dBuffers.Shutdown();
}


void Sprites3DInitialise( void )
{
	Sprites3DInitialiseGraphicsDeviceResources();
}


void Sprites3DShutdown( void )
{
SpriteGroup*	pSpriteGroups = mspSpriteGroups;
SpriteGroup*	pNext;

	while( pSpriteGroups )
	{
		pNext = pSpriteGroups->mpNext;
		delete pSpriteGroups;
		pSpriteGroups = pNext;
	}
	mspSpriteGroups = NULL;

	Sprites3DReleaseGraphicsDeviceResources();

}

SpriteGroup*	 Sprites3DFindGroup( SPRITE_GROUP hGroupNum )
{
SpriteGroup*	pSpriteGroups = mspSpriteGroups;

	while( pSpriteGroups )
	{
		if ( pSpriteGroups->mhGroupNum == hGroupNum )
		{
			return( pSpriteGroups );
		}
		pSpriteGroups = pSpriteGroups->mpNext;
	}
	return( NULL );
}

void	Sprites3DFreeGroup( SPRITE_GROUP hGroup )
{
SpriteGroup*	pSpriteGroups = mspSpriteGroups;
SpriteGroup*	pLast = NULL;

	while( pSpriteGroups )
	{
		if ( pSpriteGroups->mhGroupNum == hGroup )
		{
			if ( pLast == NULL )
			{
				mspSpriteGroups = pSpriteGroups->mpNext;
			}
			else
			{
				pLast->mpNext = pSpriteGroups->mpNext;
			}
			delete pSpriteGroups;
			return;
		}
		pLast = pSpriteGroups;
		pSpriteGroups = pSpriteGroups->mpNext;
	}

}

SPRITE_GROUP	 Sprites3DGetGroup( int nTextureHandle, float fGridScale, eSpriteGroupRenderFlags nRenderFlags )
{
SpriteGroup*	pSpriteGroups = mspSpriteGroups;

	while( pSpriteGroups )
	{
		if ( ( pSpriteGroups->mhTexture == nTextureHandle ) &&
			 ( pSpriteGroups->mRenderFlags == nRenderFlags ) )
		{
			return( pSpriteGroups->mhGroupNum );
		}
		pSpriteGroups = pSpriteGroups->mpNext;
	}

	pSpriteGroups = new SpriteGroup;
	pSpriteGroups->mpNext = mspSpriteGroups;
	mspSpriteGroups = pSpriteGroups;

	pSpriteGroups->mhTexture = nTextureHandle;
	pSpriteGroups->mhGroupNum = msnNextSpriteGroupNum++;
	pSpriteGroups->mfGridScale = fGridScale;
	pSpriteGroups->mRenderFlags = nRenderFlags;
	
	return( pSpriteGroups->mhGroupNum );
}

void	Sprites3DAddSpriteScaleZ( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, ulong ulCol, int nFrameNum, int nFlags, float fScaleZ )
{
SpriteGroup* pSpriteGroup = Sprites3DFindGroup( hGroup );

	if ( pSpriteGroup )
	{
	Sprite*		pSprite = new Sprite;

		pSprite->mpNext = pSpriteGroup->mpSpriteList;
		pSpriteGroup->mpSpriteList = pSprite;

		pSprite->mxPos = *pxPos;
		pSprite->mfScale = fScale;
		pSprite->mfScaleZ = fScaleZ;
		pSprite->mulCol = ulCol;
		pSprite->mnFrameNum = nFrameNum;
	}
}


void	Sprites3DAddSprite( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, ulong ulCol, int nFrameNum, int nFlags )
{
SpriteGroup* pSpriteGroup = Sprites3DFindGroup( hGroup );

	if ( pSpriteGroup )
	{
	Sprite*		pSprite = new Sprite;

		pSprite->mpNext = pSpriteGroup->mpSpriteList;
		pSpriteGroup->mpSpriteList = pSprite;

		pSprite->mxPos = *pxPos;
		pSprite->mfScale = fScale;
		pSprite->mfScaleZ = fScale;
		pSprite->mulCol = ulCol;
		pSprite->mnFrameNum = nFrameNum;
	}
}


void	Sprites3DAddSpriteRot( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, ulong ulCol, int nFrameNum, int nFlags, float fRotation )
{
SpriteGroup* pSpriteGroup = Sprites3DFindGroup( hGroup );

	if ( pSpriteGroup )
	{
	Sprite*		pSprite = new Sprite;

		pSprite->mpNext = pSpriteGroup->mpSpriteList;
		pSpriteGroup->mpSpriteList = pSprite;

		pSprite->mxPos = *pxPos;
		pSprite->mfScale = fScale;
		pSprite->mfScaleZ = fScale;
		pSprite->mulCol = ulCol;
		pSprite->mnFrameNum = nFrameNum;
		pSprite->mfRot = fRotation;
	}

}


void	Sprites3DCreateCamFacingOffsets( void )
{
VECT	xCamRight;
VECT	xCamDown;
VECT	xCamDir;

	xCamDir = *EngineCameraGetDirection();
	xCamDown = *EngineCameraGetUpVect();

	VectCross( &xCamRight, &xCamDir, &xCamDown );
	VectScale( &xCamDown, &xCamDown, -1.0f );
	
	maxCamFacingSpriteOffsets[0].x = (xCamRight.x * -0.5f) + (xCamDown.x * -0.5f);
	maxCamFacingSpriteOffsets[0].y = (xCamRight.y * -0.5f) + (xCamDown.y * -0.5f);
	maxCamFacingSpriteOffsets[0].z = (xCamRight.z * -0.5f) + (xCamDown.z * -0.5f);

	maxCamFacingSpriteOffsets[1].x = (xCamRight.x * 0.5f) + (xCamDown.x * -0.5f);
	maxCamFacingSpriteOffsets[1].y = (xCamRight.y * 0.5f) + (xCamDown.y * -0.5f);
	maxCamFacingSpriteOffsets[1].z = (xCamRight.z * 0.5f) + (xCamDown.z * -0.5f);

	maxCamFacingSpriteOffsets[2].x = (xCamRight.x * -0.5f) + (xCamDown.x * 0.5f);
	maxCamFacingSpriteOffsets[2].y = (xCamRight.y * -0.5f) + (xCamDown.y * 0.5f);
	maxCamFacingSpriteOffsets[2].z = (xCamRight.z * -0.5f) + (xCamDown.z * 0.5f);

	maxCamFacingSpriteOffsets[3].x = (xCamRight.x * 0.5f) + (xCamDown.x * -0.5f);
	maxCamFacingSpriteOffsets[3].y = (xCamRight.y * 0.5f) + (xCamDown.y * -0.5f);
	maxCamFacingSpriteOffsets[3].z = (xCamRight.z * 0.5f) + (xCamDown.z * -0.5f);

	maxCamFacingSpriteOffsets[4].x = (xCamRight.x * 0.5f) + (xCamDown.x * 0.5f);
	maxCamFacingSpriteOffsets[4].y = (xCamRight.y * 0.5f) + (xCamDown.y * 0.5f);
	maxCamFacingSpriteOffsets[4].z = (xCamRight.z * 0.5f) + (xCamDown.z * 0.5f);

	maxCamFacingSpriteOffsets[5].x = (xCamRight.x * -0.5f) + (xCamDown.x * 0.5f);
	maxCamFacingSpriteOffsets[5].y = (xCamRight.y * -0.5f) + (xCamDown.y * 0.5f);
	maxCamFacingSpriteOffsets[5].z = (xCamRight.z * -0.5f) + (xCamDown.z * 0.5f);


	xCamDown.x = 0.0f;
	xCamDown.y = 1.0f;
	xCamDown.z = 0.0f;
	xCamRight.x = 1.0f;
	xCamRight.y = 0.0f;
	xCamRight.z = 0.0f;
	
	maxFlatSpriteOffsets[0].x = (xCamRight.x * -0.5f) + (xCamDown.x * -0.5f);
	maxFlatSpriteOffsets[0].y = (xCamRight.y * -0.5f) + (xCamDown.y * -0.5f);
	maxFlatSpriteOffsets[0].z = (xCamRight.z * -0.5f) + (xCamDown.z * -0.5f);

	maxFlatSpriteOffsets[1].x = (xCamRight.x * 0.5f) + (xCamDown.x * -0.5f);
	maxFlatSpriteOffsets[1].y = (xCamRight.y * 0.5f) + (xCamDown.y * -0.5f);
	maxFlatSpriteOffsets[1].z = (xCamRight.z * 0.5f) + (xCamDown.z * -0.5f);

	maxFlatSpriteOffsets[2].x = (xCamRight.x * -0.5f) + (xCamDown.x * 0.5f);
	maxFlatSpriteOffsets[2].y = (xCamRight.y * -0.5f) + (xCamDown.y * 0.5f);
	maxFlatSpriteOffsets[2].z = (xCamRight.z * -0.5f) + (xCamDown.z * 0.5f);

	maxFlatSpriteOffsets[3].x = (xCamRight.x * 0.5f) + (xCamDown.x * -0.5f);
	maxFlatSpriteOffsets[3].y = (xCamRight.y * 0.5f) + (xCamDown.y * -0.5f);
	maxFlatSpriteOffsets[3].z = (xCamRight.z * 0.5f) + (xCamDown.z * -0.5f);

	maxFlatSpriteOffsets[4].x = (xCamRight.x * 0.5f) + (xCamDown.x * 0.5f);
	maxFlatSpriteOffsets[4].y = (xCamRight.y * 0.5f) + (xCamDown.y * 0.5f);
	maxFlatSpriteOffsets[4].z = (xCamRight.z * 0.5f) + (xCamDown.z * 0.5f);

	maxFlatSpriteOffsets[5].x = (xCamRight.x * -0.5f) + (xCamDown.x * 0.5f);
	maxFlatSpriteOffsets[5].y = (xCamRight.y * -0.5f) + (xCamDown.y * 0.5f);
	maxFlatSpriteOffsets[5].z = (xCamRight.z * -0.5f) + (xCamDown.z * 0.5f);

	
	xCamDown.x = 0.0f;
	xCamDown.y = 0.0f;
	xCamDown.z = 1.0f;
	xCamRight.x = 1.0f;
	xCamRight.y = 0.0f;
	xCamRight.z = 0.0f;
	
	maxXAxisSpriteOffsets[0].x = (xCamRight.x * -0.5f) + (xCamDown.x * -0.5f);
	maxXAxisSpriteOffsets[0].y = (xCamRight.y * -0.5f) + (xCamDown.y * -0.5f);
	maxXAxisSpriteOffsets[0].z = (xCamRight.z * -0.5f) + (xCamDown.z * -0.5f);

	maxXAxisSpriteOffsets[1].x = (xCamRight.x * 0.5f) + (xCamDown.x * -0.5f);
	maxXAxisSpriteOffsets[1].y = (xCamRight.y * 0.5f) + (xCamDown.y * -0.5f);
	maxXAxisSpriteOffsets[1].z = (xCamRight.z * 0.5f) + (xCamDown.z * -0.5f);

	maxXAxisSpriteOffsets[2].x = (xCamRight.x * -0.5f) + (xCamDown.x * 0.5f);
	maxXAxisSpriteOffsets[2].y = (xCamRight.y * -0.5f) + (xCamDown.y * 0.5f);
	maxXAxisSpriteOffsets[2].z = (xCamRight.z * -0.5f) + (xCamDown.z * 0.5f);

	maxXAxisSpriteOffsets[3].x = (xCamRight.x * 0.5f) + (xCamDown.x * -0.5f);
	maxXAxisSpriteOffsets[3].y = (xCamRight.y * 0.5f) + (xCamDown.y * -0.5f);
	maxXAxisSpriteOffsets[3].z = (xCamRight.z * 0.5f) + (xCamDown.z * -0.5f);

	maxXAxisSpriteOffsets[4].x = (xCamRight.x * 0.5f) + (xCamDown.x * 0.5f);
	maxXAxisSpriteOffsets[4].y = (xCamRight.y * 0.5f) + (xCamDown.y * 0.5f);
	maxXAxisSpriteOffsets[4].z = (xCamRight.z * 0.5f) + (xCamDown.z * 0.5f);

	maxXAxisSpriteOffsets[5].x = (xCamRight.x * -0.5f) + (xCamDown.x * 0.5f);
	maxXAxisSpriteOffsets[5].y = (xCamRight.y * -0.5f) + (xCamDown.y * 0.5f);
	maxXAxisSpriteOffsets[5].z = (xCamRight.z * -0.5f) + (xCamDown.z * 0.5f);

}

void Sprites3DFlush( BOOL bUseZWrite )
{
SpriteGroup*	pSpriteGroups = mspSpriteGroups;

	Sprites3DCreateCamFacingOffsets();
	EngineEnableCulling( 0 );
	if ( bUseZWrite == TRUE )
	{
		EngineEnableBlend( FALSE );
		EngineEnableZWrite( TRUE );
	}
	else
	{
		EngineEnableBlend( TRUE );
		EngineEnableZWrite( FALSE );
	}

	while( pSpriteGroups )
	{
		pSpriteGroups->Render();
		pSpriteGroups = pSpriteGroups->mpNext;
	}

}
