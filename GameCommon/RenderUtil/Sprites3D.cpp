
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


std::vector<SpriteGroup*>	mspManagedSpriteGroups;

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
	// top left
	pxOffsetGroup[0].x = (pxCamRight->x * -0.5f * fRightAspect) + (pxCamDown->x * -0.5f);
	pxOffsetGroup[0].y = (pxCamRight->y * -0.5f * fRightAspect) + (pxCamDown->y * -0.5f);
	pxOffsetGroup[0].z = (pxCamRight->z * -0.5f * fRightAspect) + (pxCamDown->z * -0.5f);

	// top right
	pxOffsetGroup[1].x = (pxCamRight->x * 0.5f * fRightAspect) + (pxCamDown->x * -0.5f);
	pxOffsetGroup[1].y = (pxCamRight->y * 0.5f * fRightAspect) + (pxCamDown->y * -0.5f);
	pxOffsetGroup[1].z = (pxCamRight->z * 0.5f * fRightAspect) + (pxCamDown->z * -0.5f);

	// bottom left
	pxOffsetGroup[2].x = (pxCamRight->x * -0.5f * fRightAspect) + (pxCamDown->x * 0.5f);
	pxOffsetGroup[2].y = (pxCamRight->y * -0.5f * fRightAspect) + (pxCamDown->y * 0.5f);
	pxOffsetGroup[2].z = (pxCamRight->z * -0.5f * fRightAspect) + (pxCamDown->z * 0.5f);

	// top right
	pxOffsetGroup[3].x = (pxCamRight->x * 0.5f * fRightAspect) + (pxCamDown->x * -0.5f);
	pxOffsetGroup[3].y = (pxCamRight->y * 0.5f * fRightAspect) + (pxCamDown->y * -0.5f);
	pxOffsetGroup[3].z = (pxCamRight->z * 0.5f * fRightAspect) + (pxCamDown->z * -0.5f);

	// bottom right
	pxOffsetGroup[4].x = (pxCamRight->x * 0.5f * fRightAspect) + (pxCamDown->x * 0.5f);
	pxOffsetGroup[4].y = (pxCamRight->y * 0.5f * fRightAspect) + (pxCamDown->y * 0.5f);
	pxOffsetGroup[4].z = (pxCamRight->z * 0.5f * fRightAspect) + (pxCamDown->z * 0.5f);

	// bottom left
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

	msfSpriteOffsetsAppliedAspect = fAspectRatio;
}


class SpriteVertexCache
{
public:
	void	Update( int nFrameNum, float fGridScale, float fScale, float fScaleZ, uint32 ulCol, int nRenderFlags, VECT* pxCamDir, float fRot );

	int		mnFrameNum = -1;
	float	mfGridScale = -1.0f;
	float	mfScale = -1.0f;
	float	mfScaleZ = -1.0f;
	uint32	mulCol = 0;
	int		mnRenderFlags = -1;
	VECT	mxCamDir = { -1.0f, -1.0f, -1.0f };
	float	mfRot = -1.0f;

	ENGINEBUFFERVERTEX		maxVertexCache[8];
};



void	SpriteVertexCache::Update( int nFrameNum, float fGridScale, float fScale, float fScaleZ, uint32 ulCol, int nRenderFlags, VECT* pxCamDir, float fRot )
{
const float		fSoftenBorder = 0.2f;
	if ( ( nFrameNum != mnFrameNum ) ||
		 ( fGridScale != mfGridScale ) )
	{	
	const float		fEdgeScale = 0.5f * fSoftenBorder;
	const float		fMidPointScale = 1.0f - fEdgeScale;
		mnFrameNum = nFrameNum;
		mfGridScale = fGridScale;
		
	float		fUBase,	fVBase;
	float		fUWidth, fVHeight;
	int			nNumPerRow = (int)( 1.0f / fGridScale );

		fUBase = (mnFrameNum % nNumPerRow) * fGridScale;
		fVBase = (mnFrameNum / nNumPerRow) * fGridScale;
		fUWidth = fGridScale;
		fVHeight = fGridScale;

		ENGINEBUFFERVERTEX* pxVertexCache = maxVertexCache;
		// Vert 1 - Top left
		pxVertexCache->tu = fUBase;
		pxVertexCache->tv = fVBase;
		pxVertexCache++;
		// Vert 2 - Top Right
		pxVertexCache->tu = fUBase + fUWidth;
		pxVertexCache->tv = fVBase;
		pxVertexCache++;
		// Vert 3 - Bot Left
		pxVertexCache->tu = fUBase;
		pxVertexCache->tv = fVBase + fVHeight;
		pxVertexCache++;
		// Vert 4 - Bot Right
		pxVertexCache->tu = fUBase + fUWidth;
		pxVertexCache->tv = fVBase + fVHeight;
		pxVertexCache++;
		// Vert 5 - Inner Top Left
		pxVertexCache->tu = fUBase + (fUWidth*fEdgeScale);
		pxVertexCache->tv = fVBase + (fVHeight*fEdgeScale);
		pxVertexCache++;
		// Vert 6 - Inner Top Right
		pxVertexCache->tu = fUBase + (fUWidth*fMidPointScale);
		pxVertexCache->tv = fVBase + (fVHeight*fEdgeScale);
		pxVertexCache++;
		// Vert 7 - Inner Bot Left
		pxVertexCache->tu = fUBase + (fUWidth*fEdgeScale);
		pxVertexCache->tv = fVBase + (fVHeight*fMidPointScale);
		pxVertexCache++;
		// Vert 8 - Inner Bot Right
		pxVertexCache->tu = fUBase + (fUWidth*fMidPointScale);
		pxVertexCache->tv = fVBase + (fVHeight*fMidPointScale);
		pxVertexCache++;
	}

	if ( ulCol != mulCol )
	{
		mulCol = ulCol;
		uint32		ulSoftEdgeCol = GetColWithModifiedAlpha( mulCol, 0.0f );

		ENGINEBUFFERVERTEX* pxVertexCache = maxVertexCache;
		// Vert 1 - Top left
		pxVertexCache->color = ulSoftEdgeCol;
		pxVertexCache++;
		// Vert 2 - Top Right
		pxVertexCache->color = ulSoftEdgeCol;
		pxVertexCache++;
		// Vert 3 - Bot Left
		pxVertexCache->color = ulSoftEdgeCol;
		pxVertexCache++;
		// Vert 4 - Bot Right
		pxVertexCache->color = ulSoftEdgeCol;
		pxVertexCache++;
		// Vert 5 - Inner Top Left
		pxVertexCache->color = ulCol;
		pxVertexCache++;
		// Vert 6 - Inner Top Right
		pxVertexCache->color = ulCol;
		pxVertexCache++;
		// Vert 7 - Inner Bot Left
		pxVertexCache->color = ulCol;
		pxVertexCache++;
		// Vert 8 - Inner Bot Right
		pxVertexCache->color = ulCol;
		pxVertexCache++;
	}

	if ( ( nRenderFlags != mnRenderFlags ) ||
		 ( pxCamDir->x != mxCamDir.x ) ||
		 ( pxCamDir->y != mxCamDir.y ) ||
		 ( pxCamDir->z != mxCamDir.z ) ||
		 ( fRot != mfRot ) ||
		 ( fScale != mfScale ) ||
		 ( fScaleZ != mfScaleZ ) )
	{
	VECT*		pxSpriteOffsets = maxCamFacingSpriteOffsets;
	VECT		xRotAxis;
	VECT*		pxRotateAxis = pxCamDir;
	ENGINEMATRIX	xRotMat;
	VECT		xOffset;
	const float		fEdgeScale = fSoftenBorder;
	const float		fMidPointScale = 1.0f - fEdgeScale;

		ENGINEBUFFERVERTEX* pxVertexCache = maxVertexCache;
		mfScale = fScale;
		mfScaleZ = fScaleZ;
		mnRenderFlags = nRenderFlags;
		mxCamDir = *pxCamDir;
		mfRot = fRot;

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

		pxVertexCache = maxVertexCache;
		// Vert 1 - Top left
		xOffset.x = pxSpriteOffsets[0].x * fScale;
		xOffset.y = pxSpriteOffsets[0].y * fScale;
		xOffset.z = pxSpriteOffsets[0].z * fScaleZ;	
		VectTransform( &pxVertexCache->position, &xOffset, &xRotMat );
		pxVertexCache++;
		// Vert 2 - Top Right
		xOffset.x = pxSpriteOffsets[1].x * fScale;
		xOffset.y = pxSpriteOffsets[1].y * fScale;
		xOffset.z = pxSpriteOffsets[1].z * fScaleZ;	
		VectTransform( &pxVertexCache->position, &xOffset, &xRotMat );
		pxVertexCache++;
		// Vert 3 - Bot Left
		xOffset.x = pxSpriteOffsets[2].x * fScale;
		xOffset.y = pxSpriteOffsets[2].y * fScale;
		xOffset.z = pxSpriteOffsets[2].z * fScaleZ;	
		VectTransform( &pxVertexCache->position, &xOffset, &xRotMat );
		pxVertexCache++;
		// Vert 4 - Bot Right
		xOffset.x = pxSpriteOffsets[4].x * fScale;
		xOffset.y = pxSpriteOffsets[4].y * fScale;
		xOffset.z = pxSpriteOffsets[4].z * fScaleZ;	
		VectTransform( &pxVertexCache->position, &xOffset, &xRotMat );
		pxVertexCache++;

		fScale *= fMidPointScale;
		fScaleZ *= fMidPointScale;

		// Vert 5 - Inner Top left
		xOffset.x = pxSpriteOffsets[0].x * fScale;
		xOffset.y = pxSpriteOffsets[0].y * fScale;
		xOffset.z = pxSpriteOffsets[0].z * fScaleZ;	
		VectTransform( &pxVertexCache->position, &xOffset, &xRotMat );
		pxVertexCache++;
		// Vert 6 - Inner Top Right
		xOffset.x = pxSpriteOffsets[1].x * fScale;
		xOffset.y = pxSpriteOffsets[1].y * fScale;
		xOffset.z = pxSpriteOffsets[1].z * fScaleZ;	
		VectTransform( &pxVertexCache->position, &xOffset, &xRotMat );
		pxVertexCache++;
		// Vert 7 - Inner Bot Left
		xOffset.x = pxSpriteOffsets[2].x * fScale;
		xOffset.y = pxSpriteOffsets[2].y * fScale;
		xOffset.z = pxSpriteOffsets[2].z * fScaleZ;	
		VectTransform( &pxVertexCache->position, &xOffset, &xRotMat );
		pxVertexCache++;
		// Vert 8 - Inner Bot Right
		xOffset.x = pxSpriteOffsets[4].x * fScale;
		xOffset.y = pxSpriteOffsets[4].y * fScale;
		xOffset.z = pxSpriteOffsets[4].z * fScaleZ;	
		VectTransform( &pxVertexCache->position, &xOffset, &xRotMat );
		pxVertexCache++;

	}

}

SpriteVertexCache		mxSpriteVertexCache;

void	Sprite::RenderRotSoftEdgesComplex( MultiVertexBuffers* pxDrawBuffer, float fGridScale, eSpriteGroupRenderFlags nRenderFlags )
{

	mxSpriteVertexCache.Update( mnFrameNum, fGridScale, mfScale, mfScaleZ, mulCol, nRenderFlags, EngineCameraGetDirection(), mfRot );

	// Tri 1
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[0], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[1], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[4], &mxPos );

	// Tri 2
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[1], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[5], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[4], &mxPos );

	// Tri 3
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[1], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[3], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[5], &mxPos );

	// Tri 4
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[3], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[7], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[5], &mxPos );

	// Tri 5
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[3], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[2], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[7], &mxPos );

	// Tri 6
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[2], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[6], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[7], &mxPos );

	// Tri 7
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[2], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[0], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[6], &mxPos );

	// Tri 8
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[0], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[4], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[6], &mxPos );

	// Tri 9
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[4], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[5], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[6], &mxPos);
	// Tri 10 
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[5], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[7], &mxPos );
	pxDrawBuffer->AddEngineVertex( &mxSpriteVertexCache.maxVertexCache[6], &mxPos );

	pxDrawBuffer->FlushWhenFull( 30, TRUE );
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
	if (mRenderFlags & kSpriteRender_DestAdd )
	{
		EngineSetBlendMode( BLEND_MODE_DESTADD );
	}
	else if (mRenderFlags & kSpriteRender_DestInv)
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

int	SpriteGroup::OnRender( void )
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
			}
			
			if ( mRenderFlags & kSpriteRender_Rotated )
			{
				if ( mRenderFlags & kSpriteRender_SoftEdges )
				{
//					pSprites->RenderRotSoftEdges( &mxSprites3dBuffers, mfGridScale, mRenderFlags );		
					pSprites->RenderRotSoftEdgesComplex( &mxSprites3dBuffers, mfGridScale, mRenderFlags );		
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
	return( 0 );
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
	for ( auto pSpriteGroup : mspManagedSpriteGroups )
	{
		delete pSpriteGroups;
	}
	mspManagedSpriteGroups.clear();

	Sprites3DReleaseGraphicsDeviceResources();
	Sprites3dBufferDeleteAll();
}

SpriteGroup*	 Sprites3DFindGroup( SPRITE_GROUP hGroupNum )
{
	for ( auto pSpriteGroup : mspManagedSpriteGroups )
	{
		if ( pSpriteGroup->mhGroupNum == hGroupNum )
		{
			return( pSpriteGroup );
		}
	}
	return( NULL );
}

void	Sprites3DFreeGroup( SPRITE_GROUP hGroup )
{
	for ( auto pSpriteGroup : mspManagedSpriteGroups )
	{
		if ( pSpriteGroup->mhGroupNum == hGroup )
		{
			mspManagedSpriteGroups.erase( pSpriteGroup );
			delete pSpriteGroup;
		}
	}

}

SPRITE_GROUP	 Sprites3DGetGroup( int nTextureHandle, float fGridScale, eSpriteGroupRenderFlags nRenderFlags, int layer )
{
	for ( auto pSpriteGroup : mspManagedSpriteGroups )
	{
		if ( ( pSpriteGroup->mhTexture == nTextureHandle ) &&
			 ( pSpriteGroup->mRenderFlags == nRenderFlags ) &&
			 ( pSpriteGroup->mLayer == layer ) )
		{
			return( pSpriteGroup->mhGroupNum );
		}
	}

	msSprite3dActiveLayers[layer] = true;

	SpriteGroup* pSpriteGroups = new SpriteGroup;
	mspManagedSpriteGroups.push_back( pSpriteGroups );

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
