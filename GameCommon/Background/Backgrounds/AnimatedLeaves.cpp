
#include <math.h>
#include "StandardDef.h"
#include "Interface.h"
#include "Engine.h"

#include "AnimatedLeaves.h"


class LeafSprite
{
public:

	void	Initialise( void );

	void	Update( float fDelta );

	void	Render( void );


	void			SetNext( LeafSprite* pNext ) { mpNext = pNext; }
	LeafSprite*		GetNext( void ) { return( mpNext ); }
private:
	void	ResetType( void );

	VECT			mxPos;
	VECT			mxVel;
	float			mfAngle1;
	float			mfAngle2;
	float			mfAngleVel1;
	float			mfAngleVel2;
	float			mfAngleMod1;
	float			mfAngleMod2;

	ulong			mulCol;
	float			mfSize;
	float			mfRotation;
	float			mfRotationVel;
	int				mnRenderType;


	LeafSprite*		mpNext;
};

LeafSprite*		mpLeafSprites = NULL;

int				msahLeafSpriteRenderOverlays[4] = { NOTFOUND };
int				msahLeafSpriteRenderSubtractiveOverlays[4] = { NOTFOUND };


void	LeafSprite::ResetType( void )
{
float	fAlpha;

	if ( (rand() % 3) != 0 ) 
	{
		mnRenderType = rand() % 8;
	}
	else
	{
		if ( rand() % 2 )
		{
			mnRenderType = 5;
		}
		else
		{
			mnRenderType = 1;
		}
	}

	// If subtractive
	if ( mnRenderType > 4 )
	{
		fAlpha = FRand( 0.01f, 0.2f );
	}
	else  // additive
	{
		fAlpha = FRand( 0.1f, 0.3f );
	}
	mulCol = GetColWithModifiedAlpha( 0xffffffff, fAlpha );

	if ( rand() % 3 )
	{
		mfSize = FRand( 0.5f, 2.5f );
	}
	else
	{
		mfSize = FRand( 0.5f, 5.0f );
	}
	mfRotation = FRand( 0.0f, A360 );
	mfRotationVel = FRand( -1.0f, 1.0f );

	mfAngle1 = FRand( 0.0f, A360 );
	mfAngle2 = FRand( 0.0f, A360 );
	mfAngleVel1 = FRand( 0.1f, 0.3f );
	mfAngleVel2 = FRand( 0.1f, 0.3f );
	mfAngleMod1 = FRand( 0.5f, 3.0f );
	mfAngleMod2 = FRand( 0.5f, 3.0f );
}

void	LeafSprite::Initialise( void )
{
int		nScreenW = InterfaceGetWidth();
int		nScreenH = InterfaceGetHeight();

	mxPos.x = FRand( 0.0f, (float)nScreenW );
	mxPos.y = FRand( 0.0f, (float)nScreenH );

	mxVel.y = FRand( 0.1f, 1.0f );

	ResetType();
}

void	LeafSprite::Update( float fDelta )
{
float		fMoveSpeed = 10.0f;
int		nScreenW = InterfaceGetWidth();
int		nScreenH = InterfaceGetHeight();
float		fRotSpeed = 0.4f;
float		fAngleVelSpeed = 1.0f;
int		nPixelSize = (int)( mfSize * 256.0f * 0.2f );

	mxPos.y += mxVel.y * fMoveSpeed * fDelta;

	mfAngle1 += mfAngleVel1 * fDelta * fAngleVelSpeed; 
	mfAngle2 += mfAngleVel2 * fDelta * fAngleVelSpeed; 

	mxVel.y = 2.0f + (sinf( mfAngle1 ) * mfAngleMod1) + (cosf(mfAngle2) * mfAngleMod1) + (sinf(mfAngle1+mfAngle2) * mfAngleMod2);
	mxVel.y *= 0.4f;

	if ( (int)(mxPos.y - nPixelSize) > InterfaceGetHeight() )
	{
		mxPos.y = 0.0f - nPixelSize;
		mxPos.x = FRand( 0.0f, (float)nScreenW );
	}
	mfRotation += mfRotationVel * fDelta * fRotSpeed;
	if ( mfRotation < 0.0f )
	{
		mfRotation += A360;
	}
	else if ( mfRotation > A360 )
	{
		mfRotation -= A360;
	}
//	ResetType();

}

void	LeafSprite::Render( void )
{
int		nOverlay;
int		nPosX, nPosY;
int		nSize;
float	fGlobalAlphaMod = 0.2f;
ulong	ulCol = GetColWithModifiedAlpha( mulCol, fGlobalAlphaMod );

	if ( mnRenderType < 4 )
	{
		nOverlay = msahLeafSpriteRenderOverlays[ mnRenderType ];
	}
	else
	{
#ifdef MARMALADE
		// Marmalade doesnt do ALPHA_SUBTRACTIVE properly yet
		nOverlay = msahLeafSpriteRenderOverlays[ mnRenderType - 4 ];
#else
		nOverlay = msahLeafSpriteRenderSubtractiveOverlays[ mnRenderType - 4 ];
#endif
	}

	nSize = (int)( 64 * mfSize );
	nPosX = (int)( mxPos.x - (nSize/2) );
	nPosY = (int)( mxPos.y - (nSize/2) );

	InterfaceSprite( nOverlay, nPosX, nPosY, 1.0f, 0, ulCol, mfRotation, mfSize * 0.2f );
//	InterfaceTexturedRect( nOverlay, nPosX, nPosY, nSize, nSize, ulCol, 0.0f, 0.0f, 1.0f, 1.0f );
	
}

//-----------------------------------------------------------

void	AnimatedLeaves::OnInitialise( void )
{
	if ( mhBackgroundTexture == NOTFOUND )
	{
//		mhBackgroundTexture = InterfaceLoadTexture( "Data\\Backgrounds\\BlankGrey.png", 0 );
		mhBackgroundTexture = InterfaceLoadTexture( "Data\\Backgrounds\\Background2a.png", 0 );

		mahBackgroundSprites[0] = InterfaceLoadTexture( "Data\\Backgrounds\\Sprites\\Flower.png", 0 );
		mahBackgroundSprites[1] = InterfaceLoadTexture( "Data\\Backgrounds\\Sprites\\Leaf.png", 0 );
		mahBackgroundSprites[2] = InterfaceLoadTexture( "Data\\Backgrounds\\Sprites\\Star.png", 0 );
		mahBackgroundSprites[3] = InterfaceLoadTexture( "Data\\Backgrounds\\Sprites\\Star2.png", 0 );
	}

	int		nLoop;
	int		nNumSprites = 120;
	LeafSprite*		pLast;
	LeafSprite*		pLeaf;

	mpLeafSprites = new LeafSprite;
	pLast = mpLeafSprites;
	pLast->Initialise();

	for( nLoop = 1; nLoop < nNumSprites; nLoop++ )
	{
		pLeaf = new LeafSprite;
		pLast->SetNext( pLeaf );
		pLeaf->Initialise();
		pLast = pLeaf;
	}
	pLeaf->SetNext( NULL );

}

void	AnimatedLeaves::OnUpdate( float fDelta )
{
LeafSprite*		pLeaf = mpLeafSprites;

	while( pLeaf )
	{
		pLeaf->Update( fDelta );
		pLeaf = pLeaf->GetNext();
	}
}

void	AnimatedLeaves::OnDisplay( float fGlobalAlpha )
{
int		nBackgroundOverlay;
LeafSprite*		pLeaf;
int		nLoop;

	nBackgroundOverlay = InterfaceCreateNewTexturedOverlay( 0, mhBackgroundTexture );
	// blue
//	InterfaceTexturedRect( nBackgroundOverlay, 0, 0, InterfaceGetWidth(), InterfaceGetHeight(), 0xFFb0b0b0, 0.0f, 0.0f, 1.0f, 1.0f );
	// green
	InterfaceTexturedRect( nBackgroundOverlay, 0, 0, InterfaceGetWidth(), InterfaceGetHeight(), 0xFF50c060, 0.0f, 0.0f, 1.0f, 1.0f );
	// red
//	InterfaceTexturedRect( nBackgroundOverlay, 0, 0, InterfaceGetWidth(), InterfaceGetHeight(), 0xFFd05040, 0.0f, 0.0f, 1.0f, 1.0f );

	for( nLoop = 0; nLoop < 4; nLoop++ )
	{
		msahLeafSpriteRenderOverlays[nLoop] = InterfaceCreateNewTexturedOverlay( 1, mahBackgroundSprites[nLoop] );
//		InterfaceOverlayRenderType( msahLeafSpriteRenderOverlays[nLoop], RENDER_TYPE_ADDITIVE );
		msahLeafSpriteRenderSubtractiveOverlays[nLoop] = InterfaceCreateNewTexturedOverlay( 1, mahBackgroundSprites[nLoop] );
		InterfaceOverlayRenderType( msahLeafSpriteRenderSubtractiveOverlays[nLoop], RENDER_TYPE_ALPHA_SUBTRACTIVE );
	}

	pLeaf = mpLeafSprites;
	while( pLeaf )
	{
		pLeaf->Render();
		pLeaf = pLeaf->GetNext();
	}

	InterfaceDraw();
}

void	AnimatedLeaves::OnShutdown( void )
{
LeafSprite*		pLeaf;
LeafSprite*		pNext;

	InterfaceReleaseTexture( mhBackgroundTexture );
	mhBackgroundTexture = NOTFOUND;

	InterfaceReleaseTexture( mahBackgroundSprites[0] );
	mahBackgroundSprites[0] = NOTFOUND;
	InterfaceReleaseTexture( mahBackgroundSprites[1] );
	mahBackgroundSprites[1] = NOTFOUND;
	InterfaceReleaseTexture( mahBackgroundSprites[2] );
	mahBackgroundSprites[2] = NOTFOUND;
	InterfaceReleaseTexture( mahBackgroundSprites[3] );
	mahBackgroundSprites[3] = NOTFOUND;

	pLeaf = mpLeafSprites;
	while( pLeaf )
	{
		pNext = pLeaf->GetNext();
		delete pLeaf;
		pLeaf = pNext;
	}
	mpLeafSprites = NULL;
}
