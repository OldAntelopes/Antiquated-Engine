
#include "StandardDef.h"
#include "Engine.h"

#include "../../../GameCommon/RenderUtil/Sprites3d.h"
#include "RenderEffects.h"

int		mshRenderEffectsSmokeTexture = NOTFOUND;

class RenderEffectParticle
{
public:
	void		InitParticle( int nType, const VECT* pxPos, const VECT* pxVel, float fScale, float fExpiryTime );

	BOOL	Update( float fDelta );
	void	Render( void );

	RenderEffectParticle*		GetNext( void ) { return( mpNext ); }
	void						SetNext( RenderEffectParticle* pNext ) { mpNext = pNext; }
private:
	int		mnType;
	VECT	mxPos;
	VECT	mxVel;
	float	mfScale;
	float	mfLifetime;
	float	mfRotation;

	float	mfExpiryTime;

	RenderEffectParticle*		mpNext;
	
};

RenderEffectParticle*		mspRenderEffectParticleList = NULL;


void	RenderEffectParticle::InitParticle( int nType, const VECT* pxPos, const VECT* pxVel, float fScale, float fExpiryTime )
{
	mnType = nType;
	mxPos = *pxPos;
	mxVel = *pxVel;
	mfScale = fScale;
	mfLifetime = 0.0f;
	mfExpiryTime = fExpiryTime;
	mfRotation = FRand( 0.0f, A360 );
}

BOOL	RenderEffectParticle::Update( float fDelta )
{
	mxPos.x += (mxVel.x * fDelta);
	mxPos.y += (mxVel.y * fDelta);
	mxPos.z += (mxVel.z * fDelta);

	mfLifetime += fDelta;

	if ( mfLifetime < 3.0f )
	{
		mfScale += fDelta * 0.25f;
	}

	if ( mfLifetime >= mfExpiryTime )
	{
		// Return FALSE means this particle gets deleted
		return( FALSE );
	}
	return( TRUE );
}

void	RenderEffectParticle::Render( void )
{
int		hSpriteGroup = Sprites3DGetGroup( mshRenderEffectsSmokeTexture, 1.0f, (eSpriteGroupRenderFlags)( kSpriteRender_Default | kSpriteRender_Rotated ) );
ulong	ulCol = 0x80ffffff;
float	fAlpha = 1.0f;
float	fTimeLeft = mfExpiryTime - mfLifetime;

	if ( mfLifetime < 1.0f )
	{
		fAlpha  = mfLifetime;
	}
	else if ( fTimeLeft < 1.0f )
	{
		fAlpha = fTimeLeft;
	}

	ulCol = GetColWithModifiedAlpha( ulCol, fAlpha );
	Sprites3DAddSpriteRot( hSpriteGroup, &mxPos, mfScale, ulCol, 0, 0, mfRotation );
	
}


//------------------------------------------------------------------------------------

void		RenderEffectsInitStaticGraphics( void )
{
	mshRenderEffectsSmokeTexture = EngineLoadTexture( "Data\\Smoke.png", 0, NULL );
}

void		RenderEffectsFreeStaticGraphics( void )
{
	EngineReleaseTexture( &mshRenderEffectsSmokeTexture );
}


void		RenderEffectsInit( void )
{
	RenderEffectsInitStaticGraphics();
}

void		RenderEffectsUpdate( float fDelta )
{
RenderEffectParticle*		pParticles = mspRenderEffectParticleList;
RenderEffectParticle*		pNext = NULL;
RenderEffectParticle*		pLast = NULL;

	while( pParticles )
	{
		pNext = pParticles->GetNext();
		if ( pParticles->Update( fDelta ) == FALSE )
		{
			if ( pLast == NULL )
			{
				mspRenderEffectParticleList = pNext;
			}
			else
			{
				pLast->SetNext( pNext );
			}
			delete pParticles;
		}
		else
		{
			pLast = pParticles;
		}
		pParticles = pNext;
	}


}


void		RenderEffectsRender( void )
{
RenderEffectParticle*		pParticles = mspRenderEffectParticleList;

	while( pParticles )
	{
		pParticles->Render();

		pParticles = pParticles->GetNext();
	}

}

void		RenderEffectsAddParticle( int nParticleType, const VECT* pxPos )
{
RenderEffectParticle*		pParticle;
float	fScale = 0.25f;
VECT	xVel = { 0.0f, 0.0f, 0.0f };
float	fExpiryTime = 5.0f;

	switch( nParticleType )
	{
	case 0:
	default:
		xVel.x = FRand( -0.1f, 0.1f );
		xVel.y = FRand( -0.1f, 0.1f );
		xVel.z = FRand( 0.4f, 0.6f );
		break;
	}
	
	pParticle = new RenderEffectParticle;
	pParticle->InitParticle( nParticleType, pxPos, &xVel, fScale, fExpiryTime );

	pParticle->SetNext( mspRenderEffectParticleList );
	mspRenderEffectParticleList = pParticle;
}

void		RenderEffectsFree( void )
{
	RenderEffectsFreeStaticGraphics();
}



/*
MVECT	xWorldPos;
float	fVertSpeedMod = 0.02f;
float	fVertSpeedBase = 0.02f;
VECT	xVel;
DYNAMIC_OBJECT*		pxNewObject;

	switch( nType )
	{
	case 0:
		xVel.fX = (( (FLOAT)(MyRand() % 64) ) - 32) / 32;
		xVel.fY = (( (FLOAT)(MyRand() % 64) ) - 32) / 32;
		xVel.fZ = ( (FLOAT)(MyRand() % 32) ) / 32;
		xVel.fX *= 0.005f;
		xVel.fY *= 0.005f;
		xVel.fZ *= fVertSpeedMod;
		xVel.fZ += fVertSpeedBase;
		pxNewObject = AddDynamicObjectToWorld( WHITE_SMOKE, (MVECT*)pxPos, &xVel, 0 );
		break;
	case 1:
		xWorldPos = *( (MVECT*)pxPos );
		xVel.fX = (float)( (MyRand()%32767) - 16384 ) / 16384;
		xVel.fY = (float)( (MyRand()%32767) - 16384 ) / 16384;
		xVel.fX *= 0.003f;
		xVel.fY *= 0.003f;
		xWorldPos.fX += xVel.fX;
		xWorldPos.fY += xVel.fY;

		xVel.fX = (float)( (MyRand()%32767) - 16384 ) / 16384;
		xVel.fY = (float)( (MyRand()%32767) - 16384 ) / 16384;
		xVel.fZ = (float)( (MyRand()%32767) - 16384 ) / 16384;

		xVel.fX *= 0.002f;
		xVel.fY *= 0.002f;
		xVel.fZ *= 0.001f;
		xVel.fZ += 0.001f;

		pxNewObject = AddDynamicObjectToWorld( DYNAMIC_OBJECT_FIRE_PARTICLE, &xWorldPos, &xVel, 4 );
		pxNewObject->ulExpiryTime = gulGlobalTime + 2000;
		pxNewObject->wParam = 1000;
		pxNewObject->wOwner = 0;
		break;
	case 2:
		xVel.fX = (( (FLOAT)(MyRand() % 64) ) - 32) / 32;
		xVel.fY = (( (FLOAT)(MyRand() % 64) ) - 32) / 32;
		xVel.fZ = ( (FLOAT)(MyRand() % 32) ) / 32;
		xVel.fX *= 0.005f;
		xVel.fY *= 0.005f;
		xVel.fZ *= fVertSpeedMod;
		xVel.fZ += fVertSpeedBase;
		pxNewObject = AddDynamicObjectToWorld( BLACK_SMOKE, (MVECT*)pxPos, &xVel, 0 );
		break;
	case 3:
		xVel.fX = (( (FLOAT)(MyRand() % 64) ) - 32) / 32;
		xVel.fY = (( (FLOAT)(MyRand() % 64) ) - 32) / 32;
		xVel.fZ = ( (FLOAT)(MyRand() % 32) ) / 32;
		xVel.fX *= 0.005f;
		xVel.fY *= 0.005f;
		xVel.fZ *= fVertSpeedMod;
		xVel.fZ += fVertSpeedBase;
		pxNewObject = AddDynamicObjectToWorld( SMOKE_1, (MVECT*)pxPos, &xVel, 0 );
		break;
	}
*/
