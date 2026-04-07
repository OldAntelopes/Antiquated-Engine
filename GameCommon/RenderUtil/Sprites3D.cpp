
#include "../LibCode/Engine/DirectX/EngineDX.h"
#include <map>
#include <vector>
#include "StandardDef.h"
#include "Engine.h"

#include "MultiVertexBuffers.h"
#include "Sprites3D.h"

#define NUM_SPRITE3D_VERTEX_BUFFERS		2
#define SPRITE3D_VERTEX_BUFFER_SIZE		4096

std::map<int,bool>		msSprite3dActiveLayers;

class Sprite
{
public:
	Sprite()
	{
		mfRot = 0.0f;
	}

	void		Render( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags );
	void		RenderRot( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags );
	void		RenderRotSoftEdges( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags );

	VECT		mxPos;
	float		mfRot;
	float		mfScale;
	float		mfScaleZ;
	float		mfAspectRatio;
	int			mnFrameNum;
	uint32		mulCol;
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
	int					mLayer;
	float				mfGridScale;
	eSpriteGroupRenderFlags	mRenderFlags;
	Sprite*				mpSpriteList;
	SpriteGroup*		mpNext;
private:
	void		ApplyRenderFlags();
};

SpriteGroup*		mspSpriteGroups = NULL;

SPRITE_GROUP		msnNextSpriteGroupNum = 9000;


MultiVertexBuffers		mxSprites3dBuffers;

float		msfSpriteOffsetsAppliedAspect = 1.0f;

VECT		maxCamFacingSpriteOffsets[6];
VECT		maxFlatSpriteOffsets[6];
VECT		maxXAxisSpriteOffsets[6];
VECT		maxYAxisSpriteOffsets[6];
VECT		mxSpriteDisplayRight;

std::vector<Sprite*>		msSprite3dFreeList;

int		mSpriteBufferSize = 0;

void	 Sprites3dBufferExpand( int nExpandAmount )
{
int		nCurrentSize = msSprite3dFreeList.capacity();

	mSpriteBufferSize += nExpandAmount;
	msSprite3dFreeList.reserve(nCurrentSize + nExpandAmount);
	for( int loop = 0; loop < nExpandAmount; loop++ )	
	{
		msSprite3dFreeList.push_back( new Sprite );
	}
}

Sprite* Sprites3dGetFreeSprite()
{
	Sprite* pSprite = NULL;
	if (msSprite3dFreeList.size() == 0 )
	{
		Sprites3dBufferExpand(16384);
	}

	if (msSprite3dFreeList.size() > 0)
	{
		pSprite = msSprite3dFreeList.back();
		msSprite3dFreeList.pop_back();
	}
	return(pSprite);
}

void Sprites3dReleaseSprite( Sprite* pSprite )
{
	msSprite3dFreeList.push_back( pSprite );
}

void	Sprites3dBufferDeleteAll()
{
	for (Sprite* pSprite : msSprite3dFreeList)
	{
		delete pSprite;
	}
	msSprite3dFreeList.clear();
}


void	Sprites3DCreateOffsetGroup( VECT* pxCamRight, VECT* pxCamDown, VECT* pxOffsetGroup, float fRightAspect )
{
	pxOffsetGroup[0].x = (pxCamRight->x * -0.5f * fRightAspect) + (pxCamDown->x * -0.5f);
	pxOffsetGroup[0].y = (pxCamRight->y * -0.5f * fRightAspect) + (pxCamDown->y * -0.5f);
	pxOffsetGroup[0].z = (pxCamRight->z * -0.5f * fRightAspect) + (pxCamDown->z * -0.5f);

	pxOffsetGroup[1].x = (pxCamRight->x * 0.5f * fRightAspect) + (pxCamDown->x * -0.5f);
	pxOffsetGroup[1].y = (pxCamRight->y * 0.5f * fRightAspect) + (pxCamDown->y * -0.5f);
	pxOffsetGroup[1].z = (pxCamRight->z * 0.5f * fRightAspect) + (pxCamDown->z * -0.5f);

	pxOffsetGroup[2].x = (pxCamRight->x * -0.5f * fRightAspect) + (pxCamDown->x * 0.5f);
	pxOffsetGroup[2].y = (pxCamRight->y * -0.5f * fRightAspect) + (pxCamDown->y * 0.5f);
	pxOffsetGroup[2].z = (pxCamRight->z * -0.5f * fRightAspect) + (pxCamDown->z * 0.5f);

	pxOffsetGroup[3].x = (pxCamRight->x * 0.5f * fRightAspect) + (pxCamDown->x * -0.5f);
	pxOffsetGroup[3].y = (pxCamRight->y * 0.5f * fRightAspect) + (pxCamDown->y * -0.5f);
	pxOffsetGroup[3].z = (pxCamRight->z * 0.5f * fRightAspect) + (pxCamDown->z * -0.5f);

	pxOffsetGroup[4].x = (pxCamRight->x * 0.5f * fRightAspect) + (pxCamDown->x * 0.5f);
	pxOffsetGroup[4].y = (pxCamRight->y * 0.5f * fRightAspect) + (pxCamDown->y * 0.5f);
	pxOffsetGroup[4].z = (pxCamRight->z * 0.5f * fRightAspect) + (pxCamDown->z * 0.5f);

	pxOffsetGroup[5].x = (pxCamRight->x * -0.5f * fRightAspect) + (pxCamDown->x * 0.5f);
	pxOffsetGroup[5].y = (pxCamRight->y * -0.5f * fRightAspect) + (pxCamDown->y * 0.5f);
	pxOffsetGroup[5].z = (pxCamRight->z * -0.5f * fRightAspect) + (pxCamDown->z * 0.5f);

}


void	Sprites3DCreateCamFacingOffsets( float fAspectRatio )
{
VECT	xCamRight;
VECT	xCamDown;
VECT	xCamDir;

	xCamDir = *EngineCameraGetDirection();
	xCamDown = *EngineCameraGetUpVect();

	VectCross( &xCamRight, &xCamDir, &xCamDown );
	VectScale( &xCamDown, &xCamDown, -1.0f );

	mxSpriteDisplayRight = xCamRight;
	Sprites3DCreateOffsetGroup( &xCamRight, &xCamDown, maxCamFacingSpriteOffsets, fAspectRatio );

	xCamDown.x = 0.0f;
	xCamDown.y = 1.0f;
	xCamDown.z = 0.0f;
	xCamRight.x = 1.0f;
	xCamRight.y = 0.0f;
	xCamRight.z = 0.0f;
	
	Sprites3DCreateOffsetGroup( &xCamRight, &xCamDown, maxFlatSpriteOffsets, fAspectRatio );

	xCamDown.x = 0.0f;
	xCamDown.y = 0.0f;
	xCamDown.z = -1.0f;
	xCamRight.x = 1.0f;
	xCamRight.y = 0.0f;
	xCamRight.z = 0.0f;

	Sprites3DCreateOffsetGroup( &xCamRight, &xCamDown, maxXAxisSpriteOffsets, fAspectRatio );

	xCamDown.x = 0.0f;
	xCamDown.y = 0.0f;
	xCamDown.z = 1.0f;
	xCamRight.x = 0.0f;
	xCamRight.y = 1.0f;
	xCamRight.z = 0.0f;

	Sprites3DCreateOffsetGroup( &xCamRight, &xCamDown, maxYAxisSpriteOffsets, fAspectRatio );

}


void	Sprite::RenderRotSoftEdges( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags )
{
float		fUBase,	fVBase;
float		fUWidth, fVHeight;
uint32		ulCol = mulCol;
VECT		xPos = mxPos;
VECT		xOffset;
int			nNumPerRow = (int)( 1.0f / fGridScale );
VECT		xRotAxis;
VECT*		pxSpriteOffsets = maxCamFacingSpriteOffsets;
VECT*		pxRotateAxis = EngineCameraGetDirection();
ENGINEMATRIX	xRotMat;
uint32		ulSoftEdgeCol = GetColWithModifiedAlpha( mulCol, 0.0f );

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
	else if ( nRenderFlags & kSpriteRender_Orientation_YAxis )
	{
		pxSpriteOffsets = maxYAxisSpriteOffsets;
		xRotAxis.x = 1.0f;
		xRotAxis.y = 0.0f;
		xRotAxis.z = 0.0f;
		pxRotateAxis = &xRotAxis;
	}
	
	EngineMatrixRotationAxis( &xRotMat, &xRotAxis, mfRot );

	fUBase = (mnFrameNum % nNumPerRow) * fGridScale;
	fVBase = (mnFrameNum / nNumPerRow) * fGridScale;
	fUWidth = fGridScale;
	fVHeight = fGridScale;

	// Tri 1 - vert 1
	*pxDrawBuffer->mpulColStream = ulSoftEdgeCol;
	xOffset.x = pxSpriteOffsets[0].x * mfScale;
	xOffset.y = pxSpriteOffsets[0].y * mfScale;
	xOffset.z = pxSpriteOffsets[0].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 1 - vert 2
	*pxDrawBuffer->mpulColStream = ulSoftEdgeCol;
	xOffset.x = pxSpriteOffsets[1].x * mfScale;
	xOffset.y = pxSpriteOffsets[1].y * mfScale;
	xOffset.z = pxSpriteOffsets[1].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 1 - vert 3 - midpoint
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	pxDrawBuffer->mpfUVStream[0] = fUBase + (fUWidth*0.5f);
	pxDrawBuffer->mpfUVStream[1] = fVBase + (fVHeight*0.5f);
	pxDrawBuffer->AddVert();
	
	//-------------------------------------------------

	// Tri 2 - vert 1
	*pxDrawBuffer->mpulColStream = ulSoftEdgeCol;
	xOffset.x = pxSpriteOffsets[3].x * mfScale;
	xOffset.y = pxSpriteOffsets[3].y * mfScale;
	xOffset.z = pxSpriteOffsets[3].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 2 - vert 2
	*pxDrawBuffer->mpulColStream = ulSoftEdgeCol;
	xOffset.x = pxSpriteOffsets[4].x * mfScale;
	xOffset.y = pxSpriteOffsets[4].y * mfScale;
	xOffset.z = pxSpriteOffsets[4].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	// Tri 2 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	pxDrawBuffer->mpfUVStream[0] = fUBase + (fUWidth*0.5f);
	pxDrawBuffer->mpfUVStream[1] = fVBase + (fVHeight*0.5f);
	pxDrawBuffer->AddVert();

	//------------------------------------
	
	// Tri 3 - vert 1
	*pxDrawBuffer->mpulColStream = ulSoftEdgeCol;
	xOffset.x = pxSpriteOffsets[4].x * mfScale;
	xOffset.y = pxSpriteOffsets[4].y * mfScale;
	xOffset.z = pxSpriteOffsets[4].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	// Tri 3 - vert 2
	*pxDrawBuffer->mpulColStream = ulSoftEdgeCol;
	xOffset.x = pxSpriteOffsets[5].x * mfScale;
	xOffset.y = pxSpriteOffsets[5].y * mfScale;
	xOffset.z = pxSpriteOffsets[5].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	// Tri 3 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	pxDrawBuffer->mpfUVStream[0] = fUBase + (fUWidth*0.5f);
	pxDrawBuffer->mpfUVStream[1] = fVBase + (fVHeight*0.5f);
	pxDrawBuffer->AddVert();

	//---------------------------
	// Tri 4 - vert 1
	*pxDrawBuffer->mpulColStream = ulSoftEdgeCol;
	xOffset.x = pxSpriteOffsets[2].x * mfScale;
	xOffset.y = pxSpriteOffsets[2].y * mfScale;
	xOffset.z = pxSpriteOffsets[2].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	// Tri 4 - vert 2
	*pxDrawBuffer->mpulColStream = ulSoftEdgeCol;
	xOffset.x = pxSpriteOffsets[0].x * mfScale;
	xOffset.y = pxSpriteOffsets[0].y * mfScale;
	xOffset.z = pxSpriteOffsets[0].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 4 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	*pxDrawBuffer->mpxPosStream = xPos;
	pxDrawBuffer->mpfUVStream[0] = fUBase + (fUWidth*0.5f);
	pxDrawBuffer->mpfUVStream[1] = fVBase + (fVHeight*0.5f);
	pxDrawBuffer->AddVert();

	pxDrawBuffer->FlushWhenFull( 12, TRUE );
}

void	Sprite::RenderRot( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags )
{
float		fUBase,	fVBase;
float		fUWidth, fVHeight;
uint32		ulCol = mulCol;
VECT		xPos = mxPos;
VECT		xOffset;
int			nNumPerRow = (int)( 1.0f / fGridScale );
VECT		xRotAxis;
VECT*		pxSpriteOffsets = maxCamFacingSpriteOffsets;
VECT*		pxRotateAxis = EngineCameraGetDirection();
ENGINEMATRIX	xRotMat;

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
	else if ( nRenderFlags & kSpriteRender_Orientation_YAxis )
	{
		pxSpriteOffsets = maxYAxisSpriteOffsets;
		xRotAxis.x = 1.0f;
		xRotAxis.y = 0.0f;
		xRotAxis.z = 0.0f;
		pxRotateAxis = &xRotAxis;
	}
	
	EngineMatrixRotationAxis( &xRotMat, pxRotateAxis, mfRot );

	fUBase = (mnFrameNum % nNumPerRow) * fGridScale;
	fVBase = (mnFrameNum / nNumPerRow) * fGridScale;
	fUWidth = fGridScale;
	fVHeight = fGridScale;

	// Tri 1 - vert 1
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[0].x * mfScale;
	xOffset.y = pxSpriteOffsets[0].y * mfScale;
	xOffset.z = pxSpriteOffsets[0].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 1 - vert 2
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[1].x * mfScale;
	xOffset.y = pxSpriteOffsets[1].y * mfScale;
	xOffset.z = pxSpriteOffsets[1].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 1 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[2].x * mfScale;
	xOffset.y = pxSpriteOffsets[2].y * mfScale;
	xOffset.z = pxSpriteOffsets[2].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();
	
	//-------------------------------------------------

	// Tri 2 - vert 1
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[3].x * mfScale;
	xOffset.y = pxSpriteOffsets[3].y * mfScale;
	xOffset.z = pxSpriteOffsets[3].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 2 - vert 2
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[4].x * mfScale;
	xOffset.y = pxSpriteOffsets[4].y * mfScale;
	xOffset.z = pxSpriteOffsets[4].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	// Tri 2 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[5].x * mfScale;
	xOffset.y = pxSpriteOffsets[5].y * mfScale;
	xOffset.z = pxSpriteOffsets[5].z * mfScaleZ;	
	VectTransform( &xOffset, &xOffset, &xRotMat );
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	pxDrawBuffer->FlushWhenFull( 12, TRUE );
}


void	Sprite::Render( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags )
{
float		fUBase,	fVBase;
float		fUWidth, fVHeight;
uint32		ulCol = mulCol;
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
	else if ( nRenderFlags & kSpriteRender_Orientation_YAxis )
	{
		pxSpriteOffsets = maxYAxisSpriteOffsets;
	}

	fUBase = (mnFrameNum % nNumPerRow) * fGridScale;
	fVBase = (mnFrameNum / nNumPerRow) * fGridScale;
	fUWidth = fGridScale;
	fVHeight = fGridScale;

	// Tri 1 - vert 1
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[0].x * mfScale;
	xOffset.y = pxSpriteOffsets[0].y * mfScale;
	xOffset.z = pxSpriteOffsets[0].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 1 - vert 2
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[1].x * mfScale;
	xOffset.y = pxSpriteOffsets[1].y * mfScale;
	xOffset.z = pxSpriteOffsets[1].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 1 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[2].x * mfScale;
	xOffset.y = pxSpriteOffsets[2].y * mfScale;
	xOffset.z = pxSpriteOffsets[2].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();
	
	//-------------------------------------------------

	// Tri 2 - vert 1
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[3].x * mfScale;
	xOffset.y = pxSpriteOffsets[3].y * mfScale;
	xOffset.z = pxSpriteOffsets[3].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase;
	pxDrawBuffer->AddVert();

	// Tri 2 - vert 2
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[4].x * mfScale;
	xOffset.y = pxSpriteOffsets[4].y * mfScale;
	xOffset.z = pxSpriteOffsets[4].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase + fUWidth;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	// Tri 2 - vert 3
	*pxDrawBuffer->mpulColStream = ulCol;
	xOffset.x = pxSpriteOffsets[5].x * mfScale;
	xOffset.y = pxSpriteOffsets[5].y * mfScale;
	xOffset.z = pxSpriteOffsets[5].z * mfScaleZ;	
	VectAdd( pxDrawBuffer->mpxPosStream, &xPos, &xOffset );
	pxDrawBuffer->mpfUVStream[0] = fUBase;
	pxDrawBuffer->mpfUVStream[1] = fVBase + fVHeight;
	pxDrawBuffer->AddVert();

	pxDrawBuffer->FlushWhenFull( 12, TRUE );
}

//-----------------------------------------------------------------------------
void	SpriteGroup::ApplyRenderFlags(  )
{
	if (mRenderFlags & kSpriteRender_DestInv)
	{
		EngineSetBlendMode( BLEND_MODE_DESTINV);
	}
	else if ( mRenderFlags & kSpriteRender_Subtractive )
	{
		if ( mRenderFlags & kSpriteRender_ColourBlend )
		{
			if ( mRenderFlags & kSpriteRender_IncAlpha)
			{
				EngineSetBlendMode( BLEND_MODE_COLOUR_SUBTRACTIVE_ALPHA );				
			}
			else
			{
				EngineSetBlendMode( BLEND_MODE_COLOUR_SUBTRACTIVE );		
			}
		}
		else
		{
			EngineSetBlendMode( BLEND_MODE_ALPHA_SUBTRACTIVE );
		}
	}
	else if ( mRenderFlags & kSpriteRender_Additive )
	{
		if ( mRenderFlags & kSpriteRender_ColourBlend )
		{
			EngineSetBlendMode( BLEND_MODE_COLOUR_ADDITIVE );		
		}
		else
		{
			EngineSetBlendMode( BLEND_MODE_SRCALPHA_ADDITIVE );
		}
	}
	else if ( mRenderFlags & kSpriteRender_ColourBlend )
	{
		if ( mRenderFlags & kSpriteRender_IncAlpha)
		{
			EngineSetBlendMode( BLEND_MODE_COLOUR_BOTHALPHA );		
		}
		else
		{
			EngineSetBlendMode( BLEND_MODE_COLOUR_BLEND );
		}
	}
	else if ( mRenderFlags & kSpriteRender_SingleColTexAlpha )
	{
		EngineSetBlendMode( BLEND_MODE_COLOUR_INVALPHA );
	}
	else
	{
		EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
	}
}

void	SpriteGroup::Render( void )
{
Sprite*		pSprites = mpSpriteList;
Sprite*		pNext;

	// DONT render if the texture hasn't loaded yet. We'd prefer nothing than big white squares
	if ( EngineTextureIsFullyLoaded( mhTexture ) == TRUE )
	{
		// Colour is texture * diffuse
		EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
		EngineSetTexture( 0, mhTexture );

		// Set blend mode
		ApplyRenderFlags();
	
		mxSprites3dBuffers.Lock();

		u64 ullEventID = SysProfileStartEvent( "SpriteGroup::Render", mhGroupNum );		
		float		fAspectRatio = 1.0f;

		while( pSprites )
		{
			pNext = pSprites->mpNext;

			if ( mRenderFlags & kSpriteRender_CustomAspect )
			{
				fAspectRatio = pSprites->mfAspectRatio;
			}
			else
			{
				fAspectRatio = 1.0f;
			}

			if ( fAspectRatio != msfSpriteOffsetsAppliedAspect )
			{
				Sprites3DCreateCamFacingOffsets( fAspectRatio );
				msfSpriteOffsetsAppliedAspect = fAspectRatio;
			}
			
			if ( mRenderFlags & kSpriteRender_Rotated )
			{
				if ( mRenderFlags & kSpriteRender_SoftEdges )
				{
					pSprites->RenderRotSoftEdges( &mxSprites3dBuffers, mfGridScale, mRenderFlags );		
				}
				else
				{
					pSprites->RenderRot( &mxSprites3dBuffers, mfGridScale, mRenderFlags );
				}
			}
			else
			{
				pSprites->Render( &mxSprites3dBuffers, mfGridScale, mRenderFlags );
			}
			// Might be wiser to pre-allocate sprites and reuse them rather than delete and re-new every frame, but this is simpler for now
			Sprites3dReleaseSprite( pSprites );
			pSprites = pNext;
		}
	
		mxSprites3dBuffers.FlushWhenFull( 0, FALSE );
		SysProfileEndEvent( ullEventID );		
	}
	else  // Texture not loaded.. just discard all the sprites for now
	{
		while( pSprites )
		{
			pNext = pSprites->mpNext;
			Sprites3dReleaseSprite( pSprites );
			pSprites = pNext;
		}			
	}

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
	Sprites3dBufferExpand( 16384 );
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
	Sprites3dBufferDeleteAll();
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

SPRITE_GROUP	 Sprites3DGetGroup( int nTextureHandle, float fGridScale, eSpriteGroupRenderFlags nRenderFlags, int layer )
{
SpriteGroup*	pSpriteGroups = mspSpriteGroups;

	while( pSpriteGroups )
	{
		if ( ( pSpriteGroups->mhTexture == nTextureHandle ) &&
			 ( pSpriteGroups->mRenderFlags == nRenderFlags ) &&
			 ( pSpriteGroups->mLayer == layer ) )
		{
			return( pSpriteGroups->mhGroupNum );
		}
		pSpriteGroups = pSpriteGroups->mpNext;
	}

	msSprite3dActiveLayers[layer] = true;

	pSpriteGroups = new SpriteGroup;
	pSpriteGroups->mpNext = mspSpriteGroups;
	mspSpriteGroups = pSpriteGroups;

	pSpriteGroups->mhTexture = nTextureHandle;
	pSpriteGroups->mLayer = layer;
	pSpriteGroups->mhGroupNum = msnNextSpriteGroupNum++;
	pSpriteGroups->mfGridScale = fGridScale;
	pSpriteGroups->mRenderFlags = nRenderFlags;
	
	return( pSpriteGroups->mhGroupNum );
}

void	Sprites3DAddSpriteScaleZ( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags, float fScaleZ )
{
SpriteGroup* pSpriteGroup = Sprites3DFindGroup( hGroup );

	if ( pSpriteGroup )
	{
	Sprite*		pSprite = Sprites3dGetFreeSprite();

		pSprite->mpNext = pSpriteGroup->mpSpriteList;
		pSpriteGroup->mpSpriteList = pSprite;

		pSprite->mxPos = *pxPos;
		pSprite->mfScale = fScale;
		pSprite->mfScaleZ = fScaleZ;
		pSprite->mulCol = ulCol;
		pSprite->mnFrameNum = nFrameNum;
	}
}


void	Sprites3DAddSprite( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags )
{
SpriteGroup* pSpriteGroup = Sprites3DFindGroup( hGroup );

	if ( pSpriteGroup )
	{
	Sprite*		pSprite = Sprites3dGetFreeSprite();

		pSprite->mpNext = pSpriteGroup->mpSpriteList;
		pSpriteGroup->mpSpriteList = pSprite;

		pSprite->mxPos = *pxPos;
		pSprite->mfScale = fScale;
		pSprite->mfScaleZ = fScale;
		pSprite->mulCol = ulCol;
		pSprite->mnFrameNum = nFrameNum;
	}
}

void	Sprites3DAddSpriteRotScaleXY( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags, float fRotation, float fXYScaleFactor )
{
SpriteGroup* pSpriteGroup = Sprites3DFindGroup( hGroup );

	if ( pSpriteGroup )
	{
	Sprite*		pSprite = Sprites3dGetFreeSprite();

		pSprite->mpNext = pSpriteGroup->mpSpriteList;
		pSpriteGroup->mpSpriteList = pSprite;

		pSprite->mxPos = *pxPos;
		pSprite->mfScale = fScale;
		pSprite->mfAspectRatio = fXYScaleFactor;
		pSprite->mfScaleZ = fScale;
		pSprite->mulCol = ulCol;
		pSprite->mnFrameNum = nFrameNum;
		pSprite->mfRot = fRotation;
	}
}

void	Sprites3DAddSpriteRot( SPRITE_GROUP hGroup, const VECT* pxPos, float fScale, uint32 ulCol, int nFrameNum, uint32 nFlags, float fRotation )
{
SpriteGroup* pSpriteGroup = Sprites3DFindGroup( hGroup );

	if ( pSpriteGroup )
	{
	Sprite*		pSprite = Sprites3dGetFreeSprite();

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



void Sprites3DCommonRenderSetup( BOOL bUseZWrite )
{
	Sprites3DCreateCamFacingOffsets( 1.0f );
	EngineEnableCulling( 0 );

	if (bUseZWrite == TRUE)
	{
		EngineEnableBlend(FALSE);
		EngineEnableZWrite(TRUE);
	}
	else
	{
		EngineEnableBlend(TRUE);
		EngineEnableZWrite(FALSE);
	}
}

void Sprites3DFlushLayer( int nLayerNum, BOOL bUseZWrite )
{
SpriteGroup*	pSpriteGroups = mspSpriteGroups;
	
	Sprites3DCommonRenderSetup(bUseZWrite);

	while( pSpriteGroups )
	{
		if ( pSpriteGroups->mLayer == nLayerNum )
		{
			pSpriteGroups->Render();
		}
		pSpriteGroups = pSpriteGroups->mpNext;
	}
	mpEngineDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	msSprite3dActiveLayers[nLayerNum] = false;
}

void Sprites3DFlush( BOOL bUseZWrite )
{
SpriteGroup*	pSpriteGroups = mspSpriteGroups;

	Sprites3DCommonRenderSetup(bUseZWrite);

	for (auto& layerActive : msSprite3dActiveLayers)
	{
		pSpriteGroups = mspSpriteGroups;
		while( pSpriteGroups )
		{
			if ( pSpriteGroups->mLayer == layerActive.first )
			{
				pSpriteGroups->Render();
			}
			pSpriteGroups = pSpriteGroups->mpNext;
		}
	}

	mpEngineDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
}
