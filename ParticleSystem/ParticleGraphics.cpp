
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "ParticleGraphics.h"

class ParticleGraphic
{
public:
	ParticleGraphic();
	~ParticleGraphic();

	void			Init( const char* szSpriteTextureName, float fGridScale, BOOL bUseRotation, eSpriteGroupRenderFlags renderFlags );

	const char*		GetTextureName( void ) { return( mszTextureName ); }

	int				GetTextureHandle( void ) { return( mhTexture ); }

	void				SetParticleGraphicID( int nID ) { mnParticleGraphicID = nID; }
	int					GetParticleGraphicID( void ) { return( mnParticleGraphicID ); }

	SPRITE_GROUP		GetSpriteGroup( void ) { return( mhSpriteGroup ); }

	ParticleGraphic*		GetNext( void ) { return( mpNext ); }
	void					SetNext( ParticleGraphic* pNext ) { mpNext = pNext; }
private:
	
	int					mhTexture;

	char*				mszTextureName;

	SPRITE_GROUP		mhSpriteGroup;
	int					mnParticleGraphicID;

	ParticleGraphic*		mpNext;

};

ParticleGraphic*		mspParticleGraphics = NULL;
int					msnNextParticleGraphicID = 0x100;


ParticleGraphic::ParticleGraphic()
{
	mszTextureName = NULL;
	mhTexture = NOTFOUND;
	mpNext = NULL;
}

ParticleGraphic::~ParticleGraphic()
{
	if ( mszTextureName )
	{
		free( mszTextureName );
	}

	if ( mhTexture != NOTFOUND )
	{
		EngineReleaseTexture( &mhTexture );
	}
}

void		ParticleGraphic::Init( const char* szTextureName, float fGridScale, BOOL bUseRotation, eSpriteGroupRenderFlags renderFlags )
{
	mszTextureName = (char*)( malloc( strlen( szTextureName ) + 1 ) );
	strcpy( mszTextureName, szTextureName );
	
	mhTexture = EngineLoadTexture( szTextureName, 0, 0 );

	if ( bUseRotation )
	{
		// todo
//		mhSpriteGroup = Sprites3DGetGroup( mhTexture, fGridScale, kSpriteRender_Additive | kSpr );
	}
	else
	{
		mhSpriteGroup = Sprites3DGetGroup( mhTexture, fGridScale, renderFlags );// kSpriteRender_Additive );
	}
}


ParticleGraphic*		ParticleGraphicsFindFromID( int nParticleGraphicID )
{
ParticleGraphic*		pParticleGraphic = mspParticleGraphics;

	while( pParticleGraphic )
	{
		if ( pParticleGraphic->GetParticleGraphicID() == nParticleGraphicID )
		{
			return( pParticleGraphic );
		}
		pParticleGraphic = pParticleGraphic->GetNext();
	}
	return( NULL );
}

ParticleGraphic*		ParticleGraphicsFind(const char* szTextureName )
{
ParticleGraphic*		pParticleGraphic = mspParticleGraphics;

	while( pParticleGraphic )
	{
		if ( stricmp( pParticleGraphic->GetTextureName(), szTextureName ) == 0 )
		{
			return( pParticleGraphic );
		}
		 
		pParticleGraphic = pParticleGraphic->GetNext();
	}

	return( NULL );
}

int		ParticleGraphicsCreate( const char* szTextureName, float fGridScale, BOOL bUseRotation, eSpriteGroupRenderFlags renderFlags )
{
ParticleGraphic*		pParticleGraphic = ParticleGraphicsFind( szTextureName );

	if ( pParticleGraphic )
	{
		return( pParticleGraphic->GetParticleGraphicID() );
	}

	pParticleGraphic = new ParticleGraphic;
	pParticleGraphic->Init( szTextureName, fGridScale, bUseRotation, renderFlags );
	pParticleGraphic->SetParticleGraphicID( msnNextParticleGraphicID );
	msnNextParticleGraphicID++;

	pParticleGraphic->SetNext( mspParticleGraphics );
	mspParticleGraphics = pParticleGraphic;

	return( pParticleGraphic->GetParticleGraphicID() );
}

SPRITE_GROUP		ParticleGraphicsGetSpriteGroup( int nParticleGraphicID )
{
ParticleGraphic*		pParticleGraphic = ParticleGraphicsFindFromID( nParticleGraphicID );
	
	if ( pParticleGraphic )
	{
		return( pParticleGraphic->GetSpriteGroup() );
	}
	return( NOTFOUND );
}

