
#include <stdio.h>
#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#include "Trails.h"

//----------------------------------------------------------
// TrailListInternal
#define	MAX_POINTS_IN_TRAIL_LIST	48
#define	MAX_TRAIL_VERTICES		((MAX_POINTS_IN_TRAIL_LIST*2)+2)
#define	NUM_POLYS_IN_TRAIL		(((MAX_POINTS_IN_TRAIL_LIST-1)*2)+2)
#define	NUM_TRAIL_INDICES		(NUM_POLYS_IN_TRAIL*3)

class TrailListInternal
{
public:
	typedef struct
	{
		VECT	xPos;
		uint32	ulTimeAdded;
		BOOL	mbIsVisible;
	} TRAIL_POINT;

	TrailListInternal()
	{
		mnType = 0;
		mulDecayTime = 7500;
		mfScale = 0.1f;
		mhTrailVertexBuffer = NOTFOUND;
		mhTrailIndexBuffer = NOTFOUND;
		mbWantsDelete = FALSE;
		mbIsAlive = TRUE;
		mfAlpha = 0.5f;

		mnNextTrailPoint = 0;
		mulLastInternalAddTick = 0;

		ZeroMemory( axTrailListInternal, sizeof(TRAIL_POINT) * MAX_POINTS_IN_TRAIL_LIST );

		mxCurrentPos.x = 0.0f;
		mxCurrentPos.y = 0.0f;
		mxCurrentPos.z = 0.0f;
		mbDeleteImmediately = FALSE;
		mnTrailHandle = NOTFOUND;
		mpNext = NULL;

	}

	~TrailListInternal()
	{
		FreeVertexBuffer();
	}

	void	Initialise( int nType, TRAIL_HANDLE nTrailHandle );

	void	Update( const VECT* pxCurrentPos, uint32 ulPointGap, BOOL bDoDraw = TRUE );
	int		Render( void );

	void	SetScale( float fScale ) { mfScale = fScale; }
	void	SetAlpha( float fAlpha ) { mfAlpha = fAlpha; }
	void	SetDecayTime( uint32 ulTime ) { mulDecayTime = ulTime; }
	void	RequestDelete( BOOL bImmediately ) { mbWantsDelete = TRUE; mbDeleteImmediately = bImmediately; }

	TRAIL_HANDLE	GetHandle( void ) { return( mnTrailHandle ); }

	BOOL	WantsDelete( void ) { return( mbWantsDelete ); }
	BOOL	IsAlive( void ) { return( mbIsAlive ); }
	BOOL	WantsImmediateDelete( void ) { return( mbDeleteImmediately ); }


	TrailListInternal*		GetNext( void ) { return( mpNext ); }
	void			SetNext( TrailListInternal* pNext ) { mpNext = pNext; }
	
	void	Reset( const VECT* pxIn )
	{
	int		nLoop;

		mnNextTrailPoint = 0;
		mulLastInternalAddTick = 0;
		ZeroMemory( axTrailListInternal, sizeof(TRAIL_POINT) * MAX_POINTS_IN_TRAIL_LIST );
		if ( pxIn )
		{
			for ( nLoop = 0; nLoop < MAX_POINTS_IN_TRAIL_LIST; nLoop++ )
			{
				AddPos( pxIn, FALSE );
			}
		}
	}
private:


	uint32	GetColour( int nIndex );

	void	AddPos( const VECT* pxIn, BOOL bDoDraw = TRUE );
	BOOL	GetPos( int nIndex, VECT* pxOut );

	void	InitTrailBuffers( void );
	void	FreeVertexBuffer(void);
	void	CalcTrailUVs( int nType );
	void	UpdateInternal( void );

	TRAIL_POINT		axTrailListInternal[MAX_POINTS_IN_TRAIL_LIST];
	int		mnNextTrailPoint;
	uint32	mulLastInternalAddTick;
	int		mnType;
	VECT	mxCurrentPos;
	float	mfScale;
	float	mfAlpha;
	uint32	mulDecayTime;

	int		mhTrailVertexBuffer;
	int		mhTrailIndexBuffer;

	BOOL			mbWantsDelete;
	BOOL			mbDeleteImmediately;
	BOOL			mbIsAlive;

	TRAIL_HANDLE	mnTrailHandle;
	TrailListInternal*		mpNext;
};

//--------------------------------------------------------------------------------------------

TrailListInternal*		mspTrails = NULL;
int				mshTrailTextureHandle = 0;
TRAIL_HANDLE	msnNextTrailHandle = 100;

void	TrailListInternal::CalcTrailUVs( int nType )
{ 
ENGINEBUFFERVERTEX*	pxVertices;
float	fBaseV = 0.004f;
int		nLoop;
BOOL	bSwitch = FALSE;

	EngineVertexBufferLock( mhTrailVertexBuffer, TRUE );
	pxVertices = EngineVertexBufferGetBufferPointer( mhTrailVertexBuffer, MAX_TRAIL_VERTICES );
	if ( pxVertices )
	{
		fBaseV += ((nType%8) * 0.125f);
		for ( nLoop = 0; nLoop < MAX_TRAIL_VERTICES; nLoop++ )
		{
			pxVertices->color = 0xFFFFFFFF;
			if ( bSwitch )
			{
				pxVertices->tv = fBaseV + 0.120f;
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


uint32	TrailListInternal::GetColour( int nIndex )
{
	if ( nIndex < MAX_POINTS_IN_TRAIL_LIST )
	{
	uint32		ulCurrentTick = SysGetTick();
	int	nActualIndex = (mnNextTrailPoint + nIndex) % MAX_POINTS_IN_TRAIL_LIST;
	uint32	ulAliveTime = ulCurrentTick - axTrailListInternal[nActualIndex].ulTimeAdded;
 
		if ( axTrailListInternal[nActualIndex].mbIsVisible == FALSE ) return( 0 );

		if ( ulAliveTime > 0 )
		{
		uint32	nExpiryTime;
		uint32	ulCol;
		int		nColMax = 0xD0;

			if ( mnType == 0 )
			{
				nColMax = 0xB0;
			}
			nExpiryTime = mulDecayTime;
			if ( ulAliveTime < nExpiryTime )
			{
				ulAliveTime = ((nExpiryTime-ulAliveTime) * nColMax)/nExpiryTime;
				ulCol = 0xF0000000 | (ulAliveTime<<16) | (ulAliveTime<<8) | ulAliveTime;
				ulCol = GetColWithModifiedAlpha( ulCol, mfAlpha );
				return( ulCol );
			}
		}
	}
		
	return( 0 );
}

BOOL	TrailListInternal::GetPos( int nIndex, VECT* pxOut )
{
	if ( nIndex < MAX_POINTS_IN_TRAIL_LIST )
	{
	int	nActualIndex = (mnNextTrailPoint + nIndex) % MAX_POINTS_IN_TRAIL_LIST;
		*pxOut = axTrailListInternal[nActualIndex].xPos;
		return( TRUE );
	}
	return( FALSE );
}

void	TrailListInternal::AddPos( const VECT* pxIn, BOOL bDoDraw )
{
uint32	ulCurrentTick = SysGetTick();

	axTrailListInternal[ mnNextTrailPoint ].mbIsVisible = bDoDraw;
	if ( pxIn )
	{
		axTrailListInternal[ mnNextTrailPoint ].xPos = *pxIn;
		axTrailListInternal[ mnNextTrailPoint ].ulTimeAdded = ulCurrentTick;
	}
	else
	{
		axTrailListInternal[ mnNextTrailPoint ].xPos.x = 0.0f;
		axTrailListInternal[ mnNextTrailPoint ].xPos.y = 0.0f;
		axTrailListInternal[ mnNextTrailPoint ].xPos.z = 0.0f;
		axTrailListInternal[ mnNextTrailPoint ].ulTimeAdded = 0;
	}
	mnNextTrailPoint++;
	mnNextTrailPoint %= MAX_POINTS_IN_TRAIL_LIST;
	axTrailListInternal[ mnNextTrailPoint ].mbIsVisible = FALSE;
	if ( pxIn )
	{
		axTrailListInternal[ mnNextTrailPoint ].xPos = *pxIn;
		axTrailListInternal[ mnNextTrailPoint ].ulTimeAdded = 0;
	}
	mulLastInternalAddTick = ulCurrentTick;
	mxCurrentPos = *pxIn;
}

void	TrailListInternal::UpdateInternal( void )
{
uint32		ulCurrentTick = SysGetTick();

	if ( axTrailListInternal[ mnNextTrailPoint ].ulTimeAdded != 0 )
	{
		if ( ulCurrentTick - axTrailListInternal[ mnNextTrailPoint ].ulTimeAdded > 10000 )
		{
			axTrailListInternal[ mnNextTrailPoint ].xPos.x = 0.0f;
			axTrailListInternal[ mnNextTrailPoint ].xPos.y = 0.0f;
			axTrailListInternal[ mnNextTrailPoint ].xPos.z = 0.0f;
			axTrailListInternal[ mnNextTrailPoint ].ulTimeAdded = 0;
		}
	}
}

void	TrailListInternal::Update( const VECT* pxCurrentPos, uint32 ulPointGap, BOOL bDoDraw )
{
uint32	ulCurrentTick = SysGetTick();

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


int	TrailListInternal::Render( void )
{
ENGINEBUFFERVERTEX*	pxVertices;
VECT	xPos;
VECT	xNextPos;
int		nLoop;
int		nNumPolysToDraw = -2;
int		nNumVerts = 2;
VECT	xCamDir = *EngineCameraGetDirection();
VECT	xTangent;
VECT	xRight = { 0.0f, 1.0f, 0.0f };
float	fScale = mfScale;
uint32	ulLastCol;
BOOL	bStillAlive = FALSE;

	if ( mhTrailVertexBuffer != NOTFOUND )
	{
		EngineVertexBufferLock( mhTrailVertexBuffer, FALSE );
		pxVertices = EngineVertexBufferGetBufferPointer( mhTrailVertexBuffer, MAX_TRAIL_VERTICES );
			
		for ( nLoop = 0; nLoop < (MAX_POINTS_IN_TRAIL_LIST); nLoop++ )
		{
			if ( GetPos( nLoop, &xPos ) == TRUE )
			{
				if ( ( xPos.x != 0.0f ) ||
					 ( xPos.z != 0.0f ) )
				{ 
					if ( GetPos( nLoop+1, &xNextPos ) == TRUE )
					{
						xTangent.x = xNextPos.x - xPos.x;
						xTangent.y = xNextPos.y - xPos.y;
						xTangent.z = xNextPos.z - xPos.z;
						VectNormalize( &xTangent );
						VectCross( &xRight, &xTangent, &xCamDir );
						VectNormalize( &xRight );
					}
					ulLastCol= GetColour( nLoop );
					if ( ulLastCol != 0 )
					{
						bStillAlive = TRUE;
					}
					pxVertices->color = ulLastCol;
					pxVertices->position.x = xPos.x + (xRight.x * fScale);
					pxVertices->position.y = xPos.y + (xRight.y * fScale);
					pxVertices->position.z = xPos.z + (xRight.z * fScale);
					pxVertices++;
					pxVertices->color = ulLastCol;
					pxVertices->position.x = xPos.x - (xRight.x * fScale);
					pxVertices->position.y = xPos.y - (xRight.y * fScale);
					pxVertices->position.z = xPos.z - (xRight.z * fScale);
					pxVertices++;
					nNumPolysToDraw += 2;
					nNumVerts += 2;
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
		}
		else
		{
			EngineVertexBufferUnlock( mhTrailVertexBuffer );
		}
	}
	mbIsAlive = bStillAlive;
	return( nNumPolysToDraw );
}

void	TrailListInternal::FreeVertexBuffer(void)
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

void	TrailListInternal::InitTrailBuffers( void )
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
	    }
	}
	if ( mhTrailIndexBuffer == NOTFOUND )
	{
		mhTrailIndexBuffer = EngineCreateIndexBuffer( NUM_TRAIL_INDICES, 0 );
		if ( mhTrailIndexBuffer == NOTFOUND )
	    {
//			PANIC_IF( TRUE, "Couldnt create trail index buffer");
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


void	TrailListInternal::Initialise( int nType, TRAIL_HANDLE hHandle )
{
	mnType = nType;
	mnTrailHandle = hHandle;
	InitTrailBuffers();

}

//-----------------------------------------------------------------------

void		TrailsInitialise( void )
{
	mshTrailTextureHandle = EngineLoadTexture( "Data/Textures/trails.bmp", 0, NULL );

}

void		TrailsUpdateAll( float fDelta )
{
TrailListInternal*		pTrails = mspTrails;
TrailListInternal*		pNext;
TrailListInternal*		pLast = NULL;

	while( pTrails )
	{
		pNext = pTrails->GetNext();
		if ( pTrails->WantsDelete() )
		{
			if ( ( pTrails->WantsImmediateDelete() ) ||
				 ( pTrails->IsAlive() == FALSE ) )
			{
				if ( pLast == NULL )
				{
					mspTrails = pNext;
				}
				else
				{
					pLast->SetNext( pNext );
				}
				delete pTrails;
			}
			else
			{
				pLast = pTrails;
			}
		}
		else
		{
			pLast = pTrails;
		}
		pTrails = pNext;
	}
	
}

void		TrailsRenderAll( void )
{
TrailListInternal*		pFind = mspTrails;
int				nCount = 0;
int				nNumDrawn = 0;
int				nPolysDrawn = 0;
int				nTotalPolysDrawn = 0;

//	EngineRestoreMainShaderState( FALSE );

	EngineSetVertexFormat( VERTEX_FORMAT_NORMAL );
	EngineSetTexture( 0, mshTrailTextureHandle );
	EngineEnableBlend( TRUE );
	EngineSetBlendMode( BLEND_MODE_SRCALPHA_ADDITIVE );
	EngineEnableLighting( FALSE );
	EngineEnableCulling( 0 );
	EngineEnableZWrite( FALSE );
	EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );

	while( pFind )
	{
		nPolysDrawn = pFind->Render();
		if ( nPolysDrawn > 0 )
		{
			nNumDrawn++;
			nTotalPolysDrawn += nPolysDrawn;
		}
		nCount++;
		pFind = pFind->GetNext();
	}

	EngineEnableZWrite( TRUE );
	EngineEnableCulling(1);
	EngineEnableLighting( TRUE );
//	char	acString[256]; 
//	sprintf( acString, "%d trails active, %d drawn (%d polys)", nCount, nNumDrawn, nTotalPolysDrawn );
//	InterfaceText( 1, 100, 100, acString, 0xffffffff, 0 );

}

void		TrailsShutdown( void )
{
TrailListInternal*		pTrails = mspTrails;
TrailListInternal*		pNext; 

	while( pTrails )
	{
		pNext = pTrails->GetNext();
		delete( pTrails );;
		pTrails = pNext;
	}
	mspTrails = NULL;
	EngineReleaseTexture( &mshTrailTextureHandle );

}

//-----------------------------------------------------------------
TrailListInternal*		TrailFind( TRAIL_HANDLE hTrailHandle )
{
TrailListInternal*		pFind = mspTrails;

	while( pFind )
	{
		if ( pFind->GetHandle() == hTrailHandle )
		{
			return( pFind );
		}
		pFind = pFind->GetNext();
	}
	return( NULL );
}


TRAIL_HANDLE		TrailCreate( int nMode, const VECT* pxStartPoint, int nFadeTimeMS, float fBandScale, float fAlpha )
{
TrailListInternal*		pNewTrail = new TrailListInternal;

	pNewTrail->SetNext( mspTrails );
	mspTrails = pNewTrail;

	pNewTrail->Initialise( nMode, msnNextTrailHandle );
	pNewTrail->SetDecayTime( nFadeTimeMS );
	pNewTrail->SetScale( fBandScale );
	pNewTrail->SetAlpha( fAlpha );
//	pNewTrail->Reset( pxStartPoint );
	msnNextTrailHandle++;
	return( pNewTrail->GetHandle() );
}

void		TrailUpdateEx( TRAIL_HANDLE hHandle, const VECT* pxPos, BOOL bVisible, uint32 ulUpdateIntervalMS )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		pTrail->Update( pxPos, ulUpdateIntervalMS, bVisible );
	}
}

void		TrailUpdate( TRAIL_HANDLE hHandle, const VECT* pxPos, BOOL bVisible )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		pTrail->Update( pxPos, 50, bVisible );
	}
}

void		TrailDelete( TRAIL_HANDLE hHandle, BOOL bDeleteImmediately )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		pTrail->RequestDelete( bDeleteImmediately );
	}
}
