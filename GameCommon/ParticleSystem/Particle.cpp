
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
	memset( &mxDir, 0, sizeof( mxDir ) );
	mfTimeAlive = 0.0f;
	mfSpriteScale = 1.0f;
	mnSpriteFrameNum = 0;
}

Particle::~Particle()
{

}

void	Particle::SetGraphicHandle( TEXTURE_HANDLE hTex, float fGridScale, BOOL bUseRotation, eRenderFlags renderFlags, int layer )
{
	mnParticleGraphicsNum = ParticleGraphicsCreateHandle( hTex, fGridScale, bUseRotation, renderFlags, layer );
}

void	Particle::SetGraphic( const char* szSpriteTextureName, float fGridScale, BOOL bUseRotation, eRenderFlags renderFlags, int layer )
{
	mnParticleGraphicsNum = ParticleGraphicsCreate( szSpriteTextureName, fGridScale, bUseRotation, renderFlags, layer );
}
	
void	Particle::Init( int typeID, const VECT* pxPos, const VECT* pxVel, uint32 ulCol, float fLongevity, int nInitParam, uint32 ulInitParamChannel, void* pUserObject, int nSpriteRenderLayer)
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
	mnSpriteRenderLayer = nSpriteRenderLayer;
	OnInit(nInitParam, pUserObject);
}




void	Particle::Update( float fDelta )
{
	if ( mfTimeAlive <= mfLongevity )
	{
	VECT	xVelThisFrame;

		mfTimeAlive += fDelta;

		OnUpdate( fDelta );
	
		VectScale( &xVelThisFrame, &mxVel, fDelta );
		VectAdd( &mxPos, &mxPos, &xVelThisFrame );

		mfRot += mfRotSpeed * fDelta;
		if ( mfRot > A180 ) mfRot -= A360;
		if ( mfRot < -A180 ) mfRot += A360;
	}
	else
	{
		mType = IN_MORGUE;
	}
	
}


void	Particle::AddVertices( MultiVertexBuffers* pVertexBuff, uint32 ulRenderFlags, uint32 ulCol )
{
Sprite		xSprite;

	xSprite.mfAspectRatio = mfSpriteAspect;
	xSprite.mfRot = GetRot();
	xSprite.mfScale = mfSpriteScale;
	xSprite.mfScaleZ = mfSpriteScale;		// Check this..
	xSprite.mnFrameNum = mnSpriteFrameNum;
	xSprite.mulCol = ulCol;	
	xSprite.mxPos = *GetPos();
	
	float	fGridScale = 1.0f;		// TODO - get this from the ParticleGraphics instead of assuming 1.0f

	Sprites3DCreateCamFacingOffsets( mfSpriteAspect );
	
	if ( ulRenderFlags & kRenderFlag_Rotated )
	{
		if ( ulRenderFlags & kRenderFlag_SoftEdges )
		{
//			xSprite.RenderRotSoftEdges( &mxSprites3dBuffers, fGridScale, mRenderFlags );		
			xSprite.RenderRotSoftEdgesComplex( pVertexBuff, fGridScale, ulRenderFlags );		
		}
		else
		{
			xSprite.RenderRot( pVertexBuff, fGridScale, ulRenderFlags );
		}
	}
	else
	{
		xSprite.Render( pVertexBuff, fGridScale, ulRenderFlags );
	}
}

void	Particle::DefaultRender( MultiVertexBuffers* pVertexBuff, uint32 ulRenderFlags )
{
	if ( mnParticleGraphicsNum != NOTFOUND )
	{
	SpriteGroup*		pSpriteGroup = ParticleGraphicsGetSpriteGroup( mnParticleGraphicsNum );
	float	fScale = 1.0f;
	uint32	ulCol;
	float	fAlpha = GetAlphaOverride();

		if ( fAlpha < 0.0f )
		{
			// Very shortlived particles just have alpha 1.0f
			if ( mfLongevity <= 0.1f )
			{
				fAlpha = 1.0f;
			}
			else  			// Everything else fades out after half-life by default
			{
			float	fHalfLife = mfLongevity * 0.5f;
			
				if ( fHalfLife < 0.1f )
				{
					fHalfLife = 0.1f;
				}

				fAlpha = 1.0f;

				if ( ( mfFadeInTime > 0.0f ) &&
					 ( mfTimeAlive < mfFadeInTime ) )
				{
					fAlpha = mfTimeAlive / mfFadeInTime;				
				}
				else if ( mfTimeAlive > fHalfLife  )
				{	
					fAlpha = 1.0f - ( ( mfTimeAlive - fHalfLife) / fHalfLife );
				}
			}
		}

		if ( fAlpha > 0.0f )
		{
			ulCol = mulCol;

			if ( fAlpha < 1.0f )	// TODO != ?????
			{
				ulCol = GetColWithModifiedAlpha( mulCol, fAlpha );
			}

			if ( pVertexBuff )
			{
				AddVertices( pVertexBuff, ulRenderFlags, ulCol );
			}
			else if ( pSpriteGroup )
			{
				if (mfSpriteAspect != 1.0f )
				{				
					pSpriteGroup->AddSpriteRotScaleXY( GetPos(), mfSpriteScale, ulCol, mnSpriteFrameNum, 0, GetRot(), mfSpriteAspect );			
				}
				else
				{
					pSpriteGroup->AddSpriteRot( GetPos(), mfSpriteScale, ulCol, mnSpriteFrameNum, 0, GetRot() );
				}

				RenderObjectList::GetCurrent().AddRenderObjectIfNotPresent( pSpriteGroup );
			}
		}
	}
}

void	Particle::RenderParticle( MultiVertexBuffers* pVertexBuff, uint32 ulRenderFlags )
{
	OnRenderParticle( pVertexBuff, ulRenderFlags );

	if ( UseDefaultRender() )
	{
		DefaultRender( pVertexBuff, ulRenderFlags );
	}
}


void	Particle::PreRender()
{
	OnPreRenderParticle();
}
