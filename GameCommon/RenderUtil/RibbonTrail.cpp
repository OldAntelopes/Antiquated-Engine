
#include "StandardDef.h"
#include "RibbonTrail.h"
#include "Trails.h"

RibbonTrail::RibbonTrail()
{
	mnType = 0;
	mulFadeHoldTimeMS = 1000;
	mulFadeOutTimeMS = 1000;

	mfScale = 0.1f;
	mhTrailVertexBuffer = NOTFOUND;
	mhTrailIndexBuffer = NOTFOUND;
	mbWantsDelete = FALSE;
	mbIsAlive = TRUE;
	mfAlpha = 0.5f;

	mnNextTrailPoint = 0;
	mnLastTrailPoint = NOTFOUND;
	mulLastInternalAddTick = 0;

	ZeroMemory( maxTrailListInternal, sizeof(TRAIL_POINT) * MAX_POINTS_IN_TRAIL_LIST );

	mxCurrentPos.x = 0.0f;
	mxCurrentPos.y = 0.0f;
	mxCurrentPos.z = 0.0f;
	mbDeleteImmediately = FALSE;

}

RibbonTrail::~RibbonTrail()
{
	FreeVertexBuffer();
}

void	RibbonTrail::Reset( const VECT* pxIn )
{
int		nLoop;

	mnNextTrailPoint = 0;
	mulLastInternalAddTick = 0;
	ZeroMemory( maxTrailListInternal, sizeof(TRAIL_POINT) * MAX_POINTS_IN_TRAIL_LIST );
	if ( pxIn )
	{
		for ( nLoop = 0; nLoop < MAX_POINTS_IN_TRAIL_LIST; nLoop++ )
		{
			AddPos( pxIn, FALSE );
		}
	}
}

void	RibbonTrail::CalcTrailUVs( int nType )
{ 
ENGINEBUFFERVERTEX*	pxVertices;
float	fBaseV = 0.004f;
int		nLoop;
BOOL	bSwitch = FALSE;

	EngineVertexBufferLock( mhTrailVertexBuffer, TRUE );
	pxVertices = EngineVertexBufferGetBufferPointer( mhTrailVertexBuffer, MAX_TRAIL_VERTICES );
	if ( pxVertices )
	{
#ifdef MAIN_GAME
		fBaseV += ((nType%8) * 0.125f);
#endif
		for ( nLoop = 0; nLoop < MAX_TRAIL_VERTICES; nLoop++ )
		{
			pxVertices->color = 0xFFFFFFFF;
			if ( bSwitch )
			{
#ifdef MAIN_GAME
				pxVertices->tv = fBaseV + 0.120f;
#else
				pxVertices->tv = fBaseV + 0.9f;
#endif
			}
			else
			{
				pxVertices->tv = fBaseV;
			}
			if ( nLoop & 1 )
			{
				pxVertices->tu = 1.0f;
				bSwitch = !bSwitch;
			}
			else
			{
				pxVertices->tu = 0.0f;
			}

			pxVertices++;
		}
	}
	EngineVertexBufferUnlock( mhTrailVertexBuffer );
}

VECT	RibbonTrail::GetSmoothedTangent( )
{
VECT	xSmoothed;

	VectAdd( &xSmoothed, &mxLastValidTangent, &mxPrevValidTangent );
	VectNormalize( &xSmoothed );
	return( xSmoothed );
}



uint32	RibbonTrail::GetColour( int nIndex, uint32 ulCurrentTick )
{
	if ( nIndex < MAX_POINTS_IN_TRAIL_LIST )
	{
	int	nActualIndex = (mnNextTrailPoint + nIndex) % MAX_POINTS_IN_TRAIL_LIST;
	uint32	ulAliveTime = ulCurrentTick - maxTrailListInternal[nActualIndex].ulTimeAdded;
 
		if ( maxTrailListInternal[nActualIndex].mbIsVisible == FALSE ) return( 0 );

		if ( ulAliveTime > 0 )
		{
		uint32	ulCol;
		uint32		ulMaxAliveTime = mulFadeHoldTimeMS + mulFadeOutTimeMS;

			if ( ulAliveTime < ulMaxAliveTime )
			{
			float	fFadeMod = 1.0f;

				if ( ulAliveTime > mulFadeHoldTimeMS )
				{
					fFadeMod = 1.0f - ((float)(ulAliveTime - mulFadeHoldTimeMS) / mulFadeOutTimeMS );
				}

				ulCol = 0xff000000 | maxTrailListInternal[nActualIndex].ulTintCol;
				ulCol = GetColWithModifiedAlpha( ulCol, mfAlpha * fFadeMod );
				return( ulCol );
			}
		}
	}
		
	return( 0 );
}

BOOL	RibbonTrail::GetPos( int nIndex, VECT* pxOut, VECT* pxTangent )
{
	if ( nIndex < MAX_POINTS_IN_TRAIL_LIST )
	{
	int	nActualIndex = (mnNextTrailPoint + nIndex) % MAX_POINTS_IN_TRAIL_LIST;
		*pxOut = maxTrailListInternal[nActualIndex].xPos;
		*pxTangent = maxTrailListInternal[nActualIndex].xTangent;
		return( TRUE );
	}
	return( FALSE );
}

void	RibbonTrail::AddPosImmediate( const VECT* pxIn, const VECT* pxTangent, uint32 ulTime, BOOL bIsVisible )
{
	mxPrevValidTangent = mxLastValidTangent;
	mxLastValidTangent = *pxTangent;
	
	maxTrailListInternal[ mnNextTrailPoint ].mbIsVisible = bIsVisible;
	maxTrailListInternal[ mnNextTrailPoint ].ulTintCol = mulTintCol;
	maxTrailListInternal[ mnNextTrailPoint ].xPos = *pxIn;
	maxTrailListInternal[ mnNextTrailPoint ].xTangent = *pxTangent;
	maxTrailListInternal[ mnNextTrailPoint ].ulTimeAdded = ulTime;
	mnLastTrailPoint = mnNextTrailPoint;
	mnNextTrailPoint++;
	mnNextTrailPoint %= MAX_POINTS_IN_TRAIL_LIST;

	// Now clear the next in the circular buffer
	maxTrailListInternal[ mnNextTrailPoint ].mbIsVisible = FALSE;
	if ( pxIn )
	{
		maxTrailListInternal[ mnNextTrailPoint ].xPos = *pxIn;
		maxTrailListInternal[ mnNextTrailPoint ].ulTimeAdded = 0;
	}
	mulLastInternalAddTick = ulTime;
}

void	RibbonTrail::AddMidPoint( const VECT* pxIn, uint32 ulTime, BOOL bIsVisible, int nRecurseLayer )
{
VECT*		pxLastPos = &maxTrailListInternal[ mnLastTrailPoint ].xPos;
VECT		xMidPoint;
uint32		ulMidTime;
VECT	xTangent;
float	fDot;
VECT*	pxLastTangent = &maxTrailListInternal[mnLastTrailPoint].xTangent;
uint32		ulLastPointTime = maxTrailListInternal[ mnLastTrailPoint ].ulTimeAdded;
float		fDist = VectDist( pxIn, pxLastPos );
VECT		xMidTangent;
	
	VectSub( &xTangent, pxIn, pxLastPos );
	VectNormalize( &xTangent );

	VectAdd( &xMidTangent, pxLastTangent, &xTangent );
	VectNormalize( &xMidTangent );
	
	VectScale( &xMidTangent, &xMidTangent, fDist * 0.5f );
	
	VectAdd( &xMidPoint, pxLastPos, &xMidTangent );

	ulMidTime = (ulTime + ulLastPointTime) / 2;

	VectSub( &xTangent, &xMidPoint, pxLastPos );
	VectNormalize( &xTangent );

	// Compare the tangent with the previously added tangent
	// If the difference is large, add another point midway  between this one and the last
	fDot = VectDot( pxLastTangent, &xTangent );
	if ( ( fDot < 0.9f ) &&
		 ( nRecurseLayer < 8 ) )
	{
		AddMidPoint( &xMidPoint, ulMidTime, bIsVisible, nRecurseLayer + 1 );
	}

	AddPosImmediate( &xMidPoint, &xTangent, ulMidTime, bIsVisible );
}

void	RibbonTrail::AddPos( const VECT* pxIn, BOOL bIsVisible )
{
uint32	ulCurrentTick = SysGetLastTick();
VECT	xTangent(0.0f,0.0f,0.0f);
VECT	xEmpty;

	if ( pxIn == NULL )
	{
		pxIn = &xEmpty;
	}
	else if ( mnLastTrailPoint != NOTFOUND )
	{
	VECT*		pxLastPos = &maxTrailListInternal[ mnLastTrailPoint ].xPos;
	const VECT*	pxLastTangent = &maxTrailListInternal[mnLastTrailPoint].xTangent;
	float	fDot;

		VectSub( &xTangent, pxIn, pxLastPos );
		VectNormalize( &xTangent );

		// TODO - Compare the tangent with the previously added tangent
		// If the difference is large, add another point midway  between this one and the last
		fDot = VectDot( pxLastTangent, &xTangent );

		if ( fDot < 0.9f )
		{
			AddMidPoint( pxIn, ulCurrentTick, 0, bIsVisible );
		}
	}

	AddPosImmediate( pxIn, &xTangent, ulCurrentTick, bIsVisible );
	mxCurrentPos = *pxIn;
}

void	RibbonTrail::UpdateInternal( void )
{
uint32		ulCurrentTick = SysGetLastTick();

	if ( maxTrailListInternal[ mnNextTrailPoint ].ulTimeAdded != 0 )
	{
		// Clear points after 10 seconds
		if ( ulCurrentTick - maxTrailListInternal[ mnNextTrailPoint ].ulTimeAdded > 10000 )
		{
			maxTrailListInternal[ mnNextTrailPoint ].xPos.x = 0.0f;
			maxTrailListInternal[ mnNextTrailPoint ].xPos.y = 0.0f;
			maxTrailListInternal[ mnNextTrailPoint ].xPos.z = 0.0f;
			maxTrailListInternal[ mnNextTrailPoint ].ulTimeAdded = 0;
		}
	}
}

void	RibbonTrail::Update( const VECT* pxCurrentPos, uint32 ulPointGap, BOOL bDoDraw )
{
uint32	ulCurrentTick = SysGetLastTick();

	if ( ulCurrentTick - mulLastInternalAddTick > ulPointGap )
	{
		AddPos( pxCurrentPos, bDoDraw );
	}
	if ( pxCurrentPos )
	{
		mxCurrentPos = *pxCurrentPos;
	}
	UpdateInternal();
}


// OnRender
//  Engine Render states (e.g. blend types) will have been set in the base RenderObject::ApplyRenderFlags call
//  so OnRender's job is just to churn out polys  (EngineIndexBufferRender)
int		RibbonTrail::OnRender( void )
{
ENGINEBUFFERVERTEX*	pxVertices;
VECT	xPos;
VECT	xNextPos;
int		nLoop;
int		nNumPolysToDraw = -2;
int		nNumVerts = 2;
VECT	xCamDir = *EngineCameraGetDirection();
VECT	xTangent;
VECT	xNextTangent;
VECT	xRight = { 0.0f, 1.0f, 0.0f };
VECT	xBlendRight = { 0.0f, 1.0f, 0.0f };
int		nBlendHistoryCount = 4;
VECT	axLastRight[4];
float	fScale = mfScale;
uint32	ulLastCol;
BOOL	bStillAlive = FALSE;
BOOL	bDebugColFlag = FALSE;

int		blendCycle = 0;

	for( int blendLoop = 0; blendLoop < nBlendHistoryCount; blendLoop++ )
	{
		axLastRight[blendLoop] = xRight;
	}

	if ( mhTrailVertexBuffer != NOTFOUND )
	{
	uint32	ulCurrentTick = SysGetLastTick();

		EngineVertexBufferLock( mhTrailVertexBuffer, FALSE );
		pxVertices = EngineVertexBufferGetBufferPointer( mhTrailVertexBuffer, MAX_TRAIL_VERTICES );
			
		for ( nLoop = 0; nLoop < (MAX_POINTS_IN_TRAIL_LIST-2); nLoop++ )
		{
			if ( GetPos( nLoop, &xPos, &xTangent ) == TRUE )
			{
				if ( ( xPos.x != 0.0f ) ||
					 ( xPos.z != 0.0f ) )
				{ 
					VectCross( &xRight, &xTangent, &xCamDir );
					VectNormalize( &xRight );	
					ulLastCol = GetColour( nLoop, ulCurrentTick );
					if ( ulLastCol != 0 )
					{
						bStillAlive = TRUE;

						if ( bDebugColFlag )
						{
							ulLastCol &= 0xFF000000;
							ulLastCol |= 0x0000f0;
						}
						xBlendRight = xRight;
	#ifdef TRAIL_VEC_RIGHT_BLENDING
						for( int blendLoop = 0; blendLoop < nBlendHistoryCount; blendLoop++ )
						{
							VectAdd( &xBlendRight, &xBlendRight, &axLastRight[blendLoop] );
						}
						VectScale( &xBlendRight, &xBlendRight, 1.0f / (float)nBlendHistoryCount );
	#endif

						pxVertices->color = ulLastCol;
						pxVertices->position.x = xPos.x + (xBlendRight.x * fScale);
						pxVertices->position.y = xPos.y + (xBlendRight.y * fScale);
						pxVertices->position.z = xPos.z + (xBlendRight.z * fScale);
						pxVertices++;
						pxVertices->color = ulLastCol;
						pxVertices->position.x = xPos.x - (xBlendRight.x * fScale);
						pxVertices->position.y = xPos.y - (xBlendRight.y * fScale);
						pxVertices->position.z = xPos.z - (xBlendRight.z * fScale);

						axLastRight[blendCycle++] = xRight;
						blendCycle %= 4;
						pxVertices++;
						nNumPolysToDraw += 2;
						nNumVerts += 2;
					}
				}
			}
		}

		if ( nNumPolysToDraw > 0 )
		{
			pxVertices->color = ulLastCol;
			pxVertices->position.x = mxCurrentPos.x + (xRight.x * fScale);
			pxVertices->position.y = mxCurrentPos.y + (xRight.y * fScale);
			pxVertices->position.z = mxCurrentPos.z + (xRight.z * fScale);
			pxVertices++;
			pxVertices->color = ulLastCol;
			pxVertices->position.x = mxCurrentPos.x - (xRight.x * fScale);
			pxVertices->position.y = mxCurrentPos.y - (xRight.y * fScale);
			pxVertices->position.z = mxCurrentPos.z - (xRight.z * fScale);
			pxVertices++;
			nNumPolysToDraw += 2;

			EngineVertexBufferAddVertsUsed( mhTrailVertexBuffer, nNumVerts );
			EngineVertexBufferUnlock( mhTrailVertexBuffer );

			EngineIndexBufferRender( mhTrailIndexBuffer, mhTrailVertexBuffer, nNumPolysToDraw, 0 );
			RenderObjectLog::AddStatCount(kRibbonPolys, nNumPolysToDraw);

		}
		else
		{
			EngineVertexBufferUnlock( mhTrailVertexBuffer );
		}
	}
	mbIsAlive = bStillAlive;
	return( nNumPolysToDraw );
}

void	RibbonTrail::FreeVertexBuffer(void)
{
	if ( mhTrailVertexBuffer != NOTFOUND )
	{
		EngineVertexBufferFree( mhTrailVertexBuffer );
		mhTrailVertexBuffer = NOTFOUND;
	}
	if ( mhTrailIndexBuffer != NOTFOUND )
	{
		EngineIndexBufferFree( mhTrailIndexBuffer );
		mhTrailIndexBuffer = NOTFOUND;
	}
}

void	RibbonTrail::InitTrailBuffers( void )
{
ushort*	puwIndexBuff;
int		nLoop;
int		nRow;
BOOL	bSwitch = FALSE;
float	fBaseV = 0.005f;

	if ( mhTrailVertexBuffer == NOTFOUND )
	{
		mhTrailVertexBuffer = EngineCreateVertexBuffer( MAX_TRAIL_VERTICES, 0, "Trail VB" );
	    if( mhTrailVertexBuffer == NOTFOUND )
	    {
//			PANIC_IF( TRUE, "Couldnt create trail vertex buffer");
			return;
	    }
	}

	if ( mhTrailIndexBuffer == NOTFOUND )
	{
		mhTrailIndexBuffer = EngineCreateIndexBuffer( NUM_TRAIL_INDICES, 0 );
		if ( mhTrailIndexBuffer == NOTFOUND )
	    {
//			PANIC_IF( TRUE, "Couldnt create trail index buffer");
			EngineVertexBufferFree( mhTrailVertexBuffer );
			mhTrailVertexBuffer = NOTFOUND;
			return;
	    }
	}

	puwIndexBuff = EngineIndexBufferLock( mhTrailIndexBuffer, 0 );
	nRow = 0;
	for ( nLoop = 0; nLoop < NUM_POLYS_IN_TRAIL; nLoop += 2 )
	{
		puwIndexBuff[0] = nRow*2;
		puwIndexBuff[1] = (nRow*2)+1;
		puwIndexBuff[2] = (nRow*2)+2;

		puwIndexBuff[3] = (nRow*2)+1;
		puwIndexBuff[4] = (nRow*2)+3;
		puwIndexBuff[5] = (nRow*2)+2;
		puwIndexBuff += 6;
		nRow++;
	}
	EngineIndexBufferUnlock( mhTrailIndexBuffer );

	CalcTrailUVs(mnType);
}


void	RibbonTrail::Initialise( int nType )
{
	mnType = nType;
	mulTintCol = 0xFFFFFF;
	InitTrailBuffers();

	SetTextureHandle( TrailsGetTrailFaderTextureHandle() );
	SetRenderType(kRenderType_RibbonTrail);
}
