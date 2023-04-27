
#include <math.h>
#include "StandardDef.h"
#include "Interface.h"

#include "Snowflakes.h"


class SnowSprite
{
public:

	void	Initialise( void );

	void	Update( float fDelta );

	void	Render( int nLayer );


	void			SetNext( SnowSprite* pNext ) { mpNext = pNext; }
	SnowSprite*		GetNext( void ) { return( mpNext ); }
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

	int				mnLayer;
	uint32			mulCol;
	float			mfSize;
	float			mfRotation;
	float			mfRotationVel;
	int				mnRenderType;


	SnowSprite*		mpNext;
};

SnowSprite*		mpSnowSprites = NULL;

int				msahSnowSpriteRenderOverlays[3] = { NOTFOUND };


void	SnowSprite::ResetType( void )
{
float	fAlpha;

	mnRenderType = rand() % 3;

	if ( rand() % 3 )
	{
		mfSize = FRand( 0.2f, 0.8f );
	}
	else
	{
		mfSize = FRand( 0.3f, 3.0f );
	}
	mfRotation = FRand( 0.0f, A360 );
	mfRotationVel = FRand( -1.0f, 1.0f );

	mfAngle1 = FRand( 0.0f, A360 );
	mfAngle2 = FRand( 0.0f, A360 );
	mfAngleVel1 = FRand( 0.1f, 0.3f );
	mfAngleVel2 = FRand( 0.1f, 0.3f );
	mfAngleMod1 = FRand( 0.5f, 3.0f );
	mfAngleMod2 = FRand( 0.5f, 3.0f );

	if ( rand() % 4 )
	{
		mnLayer = 0;
	}
	else
	{
		mnLayer = 1;
	}

	if ( mnLayer == 1 )
	{
		fAlpha = FRand( 0.4f, 0.7f );
	}
	else
	{
		fAlpha = FRand( 0.7f, 1.0f );
	}
	mulCol = GetColWithModifiedAlpha( 0xffffffff, fAlpha );
}

void	SnowSprite::Initialise( void )
{
int		nScreenW = InterfaceGetWidth();
int		nScreenH = InterfaceGetHeight();

	mxPos.x = FRand( 0.0f, (float)nScreenW );
	mxPos.y = FRand( 0.0f, (float)nScreenH );

	mxVel.y = FRand( 0.1f, 1.0f );

	ResetType();
}

void	SnowSprite::Update( float fDelta )
{
float		fMoveSpeed = 20.0f;
int		nScreenW = InterfaceGetWidth();
int		nScreenH = InterfaceGetHeight();
float		fRotSpeed = 0.4f;
float		fAngleVelSpeed = 1.0f;
int		nPixelSize = (int)( mfSize * 256.0f * 0.2f );

	mxPos.y += mxVel.y * fMoveSpeed * fDelta;

	mfAngle1 += mfAngleVel1 * fDelta * fAngleVelSpeed; 
	mfAngle2 += mfAngleVel2 * fDelta * fAngleVelSpeed; 

	mxVel.y = 2.5f + (sinf( mfAngle1 ) * mfAngleMod1) + (cosf(mfAngle2) * mfAngleMod1) + (sinf(mfAngle1+mfAngle2) * mfAngleMod2);
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

void	SnowSprite::Render( int nLayer )
{
	if ( nLayer == mnLayer )
	{
	int		nOverlay;
	int		nPosX, nPosY;
	int		nSize;
	float	fGlobalAlphaMod = 0.8f;
	uint32	ulCol = GetColWithModifiedAlpha( mulCol, fGlobalAlphaMod );

		nOverlay = msahSnowSpriteRenderOverlays[ mnRenderType ];

		nSize = (int)( 64 * mfSize );
		nPosX = (int)( mxPos.x - (nSize/2) );
		nPosY = (int)( mxPos.y - (nSize/2) );

		InterfaceSprite( nOverlay, nPosX, nPosY, 1.0f, 0, ulCol, mfRotation, mfSize * 0.2f );
	//	InterfaceTexturedRect( nOverlay, nPosX, nPosY, nSize, nSize, ulCol, 0.0f, 0.0f, 1.0f, 1.0f );
	}	
}

//-----------------------------------------------------------

void	Snowflakes::OnInitialise( void )
{
	if ( mhBackgroundTexture == NOTFOUND )
	{
		mhBackgroundTexture = InterfaceLoadTexture( "Data\\UI\\Backgrounds\\BlankGrey.png", 0 );

		mahBackgroundSprites[0] = InterfaceLoadTexture( "Data\\UI\\Backgrounds\\Sprites\\Snowflake1.png", 0 );
		mahBackgroundSprites[1] = InterfaceLoadTexture( "Data\\UI\\Backgrounds\\Sprites\\Snowflake2.png", 0 );
		mahBackgroundSprites[2] = InterfaceLoadTexture( "Data\\UI\\Backgrounds\\Sprites\\Snowflake3.png", 0 );
	}

	int		nLoop;
	int		nNumSprites = 120;
	SnowSprite*		pLast;
	SnowSprite*		pLeaf;

	mpSnowSprites = new SnowSprite;
	pLast = mpSnowSprites;
	pLast->Initialise();

	for( nLoop = 1; nLoop < nNumSprites; nLoop++ )
	{
		pLeaf = new SnowSprite;
		pLast->SetNext( pLeaf );
		pLeaf->Initialise();
		pLast = pLeaf;
	}
	pLeaf->SetNext( NULL );

}

void	Snowflakes::OnUpdate( float fDelta )
{
SnowSprite*		pLeaf = mpSnowSprites;

	while( pLeaf )
	{
		pLeaf->Update( fDelta );
		pLeaf = pLeaf->GetNext();
	}
}

void	Snowflakes::OnDisplayUpperLayer( void )
{
SnowSprite*		pLeaf;

	pLeaf = mpSnowSprites;
	while( pLeaf )
	{
		pLeaf->Render(1);
		pLeaf = pLeaf->GetNext();
	}

}

void	Snowflakes::OnDisplay( float fGlobalAlpha )
{
int		nBackgroundOverlay;
SnowSprite*		pLeaf;
int		nLoop;

	nBackgroundOverlay = InterfaceCreateNewTexturedOverlay( 0, mhBackgroundTexture );
	InterfaceTexturedRect( nBackgroundOverlay, 0, 0, InterfaceGetWidth(), InterfaceGetHeight(), 0xFFd05040, 0.0f, 0.0f, 1.0f, 1.0f );

	for( nLoop = 0; nLoop < 4; nLoop++ )
	{
		msahSnowSpriteRenderOverlays[nLoop] = InterfaceCreateNewTexturedOverlay( 1, mahBackgroundSprites[nLoop] );
		InterfaceOverlayRenderType( msahSnowSpriteRenderOverlays[nLoop], RENDER_TYPE_ADDITIVE );
	}

	pLeaf = mpSnowSprites;
	while( pLeaf )
	{
		pLeaf->Render(0);
		pLeaf = pLeaf->GetNext();
	}
}

void	Snowflakes::OnShutdown( void )
{
	InterfaceReleaseTexture( mhBackgroundTexture );
	mhBackgroundTexture = NOTFOUND;


}
