
#include <math.h>
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "ParticleGraphics.h"
#include "ParticleManager.h"
#include "Particle.h"

Particle::Particle()
{
	mnParticleGraphicsNum = NOTFOUND;
	memset( &mxPos, 0, sizeof( mxPos ) );
	memset( &mxVel, 0, sizeof( mxVel ) );
	mfTimeAlive = 0.0f;
	mfSpriteScale = 1.0f;
	mnSpriteFrameNum = 0;
}

Particle::~Particle()
{

}


void	Particle::SetGraphic( const char* szSpriteTextureName, float fGridScale, BOOL bUseRotation, eSpriteGroupRenderFlags renderFlags )
{
	mnParticleGraphicsNum = ParticleGraphicsCreate( szSpriteTextureName, fGridScale, bUseRotation, renderFlags );
}
	
void	Particle::Init( int typeID, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam, uint32 ulInitParamChannel )
{
	mType = typeID;
	if ( pxPos )
	{
		mxPos = *pxPos;
	}
	if ( pxVel )
	{
		mxVel = *pxVel;
	}
	mulCol = ulCol;
	mulParamChannel = ulInitParamChannel;
	mfLongevity = fLongevity;
	OnInit(nInitParam);
}




void	Particle::Update( float fDelta )
{
	mfTimeAlive += fDelta;
	if ( mfTimeAlive < mfLongevity )
	{
	VECT	xVelThisFrame;

		OnUpdate( fDelta );
	
		VectScale( &xVelThisFrame, &mxVel, fDelta );
		VectAdd( &mxPos, &mxPos, &xVelThisFrame );
	}
	else
	{
		mType = IN_MORGUE;
	}
	
}


void	Particle::DefaultRender( void )
{
	if ( mnParticleGraphicsNum != NOTFOUND )
	{
	SPRITE_GROUP		hSpriteGroup = ParticleGraphicsGetSpriteGroup( mnParticleGraphicsNum );
	float	fScale = 1.0f;
	uint32	ulCol;
	float	fAlpha = GetAlphaOverride();

		if ( fAlpha < 0.0f )
		{
		float	fHalfLife = mfLongevity * 0.5f;
			
			fAlpha = 1.0f;
		
			if ( mfTimeAlive > fHalfLife  )
			{	
				fAlpha = 1.0f - ( ( mfTimeAlive - fHalfLife) / fHalfLife );
			}
		}

		if ( fAlpha < 1.0f )
		{
			if ( fAlpha > 0.0f )
			{
				ulCol = GetColWithModifiedAlpha( mulCol, fAlpha );
				Sprites3DAddSprite( hSpriteGroup, GetPos(), mfSpriteScale, ulCol, mnSpriteFrameNum, 0 );
			}
		}
		else
		{
			Sprites3DAddSprite( hSpriteGroup, GetPos(), mfSpriteScale, mulCol, mnSpriteFrameNum, 0 );
		}
	}
}

void	Particle::Render( void )
{
	OnRender();

	if ( UseDefaultRender() )
	{
		DefaultRender();
	}

}



