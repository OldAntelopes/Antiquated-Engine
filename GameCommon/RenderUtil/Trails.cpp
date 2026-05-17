
#include <stdio.h>
#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#include "RibbonTrail.h"
#include "Trails.h"

//----------------------------------------------------------
// TrailListInternal
#define	MAX_POINTS_IN_TRAIL_LIST	512

#define	MAX_TRAIL_VERTICES		((MAX_POINTS_IN_TRAIL_LIST*2)+2)
#define	NUM_POLYS_IN_TRAIL		(((MAX_POINTS_IN_TRAIL_LIST-1)*2)+2)
#define	NUM_TRAIL_INDICES		(NUM_POLYS_IN_TRAIL*3)

class TrailListInternal
{
public:
	TrailListInternal()
	{
		mnTrailHandle = NOTFOUND;
		mpNext = NULL;
	}

	~TrailListInternal()
	{
	}

	void	Initialise( int nMode, TRAIL_HANDLE hHandle )
	{
		mnTrailHandle = hHandle;
		mRibbonTrail.Initialise( nMode );
	}

	TRAIL_HANDLE	GetHandle( void ) { return( mnTrailHandle ); }

	RibbonTrail&	GetTrail(void) { return(mRibbonTrail); }

	TrailListInternal*		GetNext( void ) { return( mpNext ); }
	void			SetNext( TrailListInternal* pNext ) { mpNext = pNext; }
	
private:

	RibbonTrail		mRibbonTrail;

	TRAIL_HANDLE	mnTrailHandle;
	TrailListInternal*		mpNext;
};

//--------------------------------------------------------------------------------------------

TrailListInternal*		mspTrails = NULL;
int				mshTrailTextureHandle = 0;
TRAIL_HANDLE	msnNextTrailHandle = 100;

//-----------------------------------------------------------------------

void		TrailsInitialise( void )
{
	mshTrailTextureHandle = EngineLoadTexture( "Data/Textures/TrailFader.png", 0, NULL );
}

void		TrailsOnGraphicDeviceChanged()
{
	TrailsInitialise();
}



int		TrailsGetTrailFaderTextureHandle( void )
{
	return( mshTrailTextureHandle );
}


void		TrailsUpdateAll( float fDelta )
{
TrailListInternal*		pTrails = mspTrails;
TrailListInternal*		pNext;
TrailListInternal*		pLast = NULL;

	while( pTrails )
	{
		pNext = pTrails->GetNext();
		if ( pTrails->GetTrail().WantsDelete() )
		{
			if ( ( pTrails->GetTrail().WantsImmediateDelete() ) ||
				 ( pTrails->GetTrail().IsAlive() == FALSE ) )
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
		nPolysDrawn = pFind->GetTrail().Render();
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

//	char	acString[256]; 
//	sprintf( acString, "%d trails active, %d drawn (%d polys)", nCount, nNumDrawn, nTotalPolysDrawn );
//	InterfaceText( 1, 100, 100, acString, 0xffffffff, 0 );
	EngineSetBlendMode( BLEND_MODE_ALPHABLEND );

}

void		TrailsShutdown( void )
{
TrailListInternal*		pTrails = mspTrails;
TrailListInternal*		pNext; 

	while( pTrails )
	{
		pNext = pTrails->GetNext();
		delete( pTrails );
		pTrails = pNext;
	}
	mspTrails = NULL;
	EngineReleaseTexture( &mshTrailTextureHandle );

}

//-----------------------------------------------------------------
TrailListInternal*		TrailFind( TRAIL_HANDLE hTrailHandle )
{
TrailListInternal*		pFind = mspTrails;

// todo - replace this with a map
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
	pNewTrail->GetTrail().SetDecayTime( nFadeTimeMS );
	pNewTrail->GetTrail().SetScale( fBandScale );
	pNewTrail->GetTrail().SetAlpha( fAlpha );
//	pNewTrail->Reset( pxStartPoint );
	msnNextTrailHandle++;
	return( pNewTrail->GetHandle() );
}

void		TrailUpdateEx( TRAIL_HANDLE hHandle, const VECT* pxPos, BOOL bVisible, uint32 ulUpdateIntervalMS )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		pTrail->GetTrail().Update( pxPos, ulUpdateIntervalMS, bVisible );
	}
}

void		TrailUpdate( TRAIL_HANDLE hHandle, const VECT* pxPos, BOOL bVisible )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		pTrail->GetTrail().Update( pxPos, 50, bVisible );
	}
}

void		TrailDelete( TRAIL_HANDLE hHandle, BOOL bDeleteImmediately )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		pTrail->GetTrail().RequestDelete( bDeleteImmediately );
	}
}

void		TrailSetScale( TRAIL_HANDLE hHandle, float fBandScale )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		pTrail->GetTrail().SetScale(fBandScale);
	}	
}

void		TrailSetAlpha( TRAIL_HANDLE hHandle, float fAlpha )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		pTrail->GetTrail().SetAlpha(fAlpha);
	}	
}

VECT		TrailGetDirection( TRAIL_HANDLE hHandle )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		return( pTrail->GetTrail().GetTangent() );
	}
	return( VECT( 0.0f, 1.0f, 0.0f ) );
}

void		TrailSetFadeProp( TRAIL_HANDLE hHandle, uint32 ulFadeHoldTimeMS, uint32 ulFadeOutTimeMS )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		pTrail->GetTrail().SetFadeProp( ulFadeHoldTimeMS, ulFadeOutTimeMS );
	}
}


void		TrailSetTint( TRAIL_HANDLE hHandle, uint32 ulTintCol )
{
TrailListInternal*		pTrail = TrailFind( hHandle );
	
	if ( pTrail )
	{
		pTrail->GetTrail().SetTint( ulTintCol );
	}	
}