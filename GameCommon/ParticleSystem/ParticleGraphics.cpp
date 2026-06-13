
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "ParticleGraphics.h"

class ParticleGraphic
{
public:
	ParticleGraphic();
	~ParticleGraphic();

	void			Init( const char* szSpriteTextureName, float fGridScale, BOOL bUseRotation, eRenderFlags renderFlags, int layer = 0 );
	void			InitFromTexHandle( int hTex, float fGridScale, BOOL bUseRotation, eRenderFlags renderFlags, int layer = 0 );

	const char*		GetTextureName( void ) { return( mszTextureName ); }

	int				GetTextureHandle( void ) { return( mhTexture ); }

	void				SetParticleGraphicID( int nID ) { mnParticleGraphicID = nID; }
	int					GetParticleGraphicID( void ) { return( mnParticleGraphicID ); }

	SpriteGroup*		GetSpriteGroup( void ) { return( mpSpriteGroup ); }
//	SPRITE_GROUP		GetSpriteGroupHandle( void ) { return( mhSpriteGroup ); }

	eRenderFlags		GetRenderFlags() const { return( mRenderFlags ); }
	int				GetLayer() const { return(mLayer); }		

	ParticleGraphic*		GetNext( void ) { return( mpNext ); }
	void					SetNext( ParticleGraphic* pNext ) { mpNext = pNext; }
private:
	
	int					mhTexture;

	char*				mszTextureName = NULL;
	eRenderFlags		mRenderFlags;

//	SPRITE_GROUP		mhSpriteGroup;
	SpriteGroup* mpSpriteGroup;

	int					mLayer;	
	int					mnParticleGraphicID;

	ParticleGraphic*		mpNext;

};

ParticleGraphic*		mspParticleGraphics = NULL;
int					msnNextParticleGraphicID = 0x100;
int					msnNumParticleGraphics = 0;

ParticleGraphic::ParticleGraphic()
{
	mszTextureName = NULL;
	mhTexture = NOTFOUND;
	mpNext = NULL;
	msnNumParticleGraphics++;
}

ParticleGraphic::~ParticleGraphic()
{
	msnNumParticleGraphics--;
	SAFE_FREE(mszTextureName);

	if ( mhTexture != NOTFOUND )
	{
		EngineReleaseTexture( &mhTexture );
	}

	mpSpriteGroup->Release();
	mpSpriteGroup = NULL;
}

void		ParticleGraphic::InitFromTexHandle( int hTex, float fGridScale, BOOL bUseRotation, eRenderFlags renderFlags, int layer )
{
const char*		szTextureName = "<External>";

	mRenderFlags = renderFlags;
	mLayer = layer;
	SAFE_FREE(mszTextureName);
	mszTextureName = (char*)( malloc( strlen( szTextureName ) + 1 ) );
	strcpy( mszTextureName, szTextureName );

	mhTexture = hTex;
//	mhSpriteGroup = Sprites3DGetManagedGroupHandle( mhTexture, fGridScale, renderFlags, layer );

	mpSpriteGroup = Sprites3DGetGroup(mhTexture, fGridScale, renderFlags, layer);	
}

void		ParticleGraphic::Init( const char* szTextureName, float fGridScale, BOOL bUseRotation, eRenderFlags renderFlags, int layer )
{
	mszTextureName = (char*)( malloc( strlen( szTextureName ) + 1 ) );
	strcpy( mszTextureName, szTextureName );
	
	mhTexture = EngineLoadTexture( szTextureName, 0, 0 );
	mRenderFlags = renderFlags;
	mLayer = layer;

	mpSpriteGroup = Sprites3DGetGroup(mhTexture, fGridScale, renderFlags, layer);	
	
//	mhSpriteGroup = Sprites3DGetManagedGroupHandle( mhTexture, fGridScale, renderFlags, layer );

}


//-------------------------------------------------------------------------------------------------
void		ParticleGraphicsInit()
{

}


void		ParticleGraphicsShutdown( )
{
ParticleGraphic*		pParticleGraphic = mspParticleGraphics;
ParticleGraphic*		pNext;

	while( pParticleGraphic )
	{
		pNext = pParticleGraphic->GetNext();
		delete pParticleGraphic;
		pParticleGraphic = pNext;
	}
	mspParticleGraphics = NULL;

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

ParticleGraphic*		ParticleGraphicsFindTexHandle( int hTex, eRenderFlags renderFlags, int layer )
{
ParticleGraphic*		pParticleGraphic = mspParticleGraphics;

	// TODO!! Need to change this to a unordered_map on TexHandle

	while( pParticleGraphic )
	{
		if ( ( pParticleGraphic->GetTextureHandle() == hTex ) &&
			 ( pParticleGraphic->GetRenderFlags() == renderFlags ) &&
			 ( pParticleGraphic->GetLayer() == layer ) )
		{
			return( pParticleGraphic );
		}
		 
		pParticleGraphic = pParticleGraphic->GetNext();
	}

	return( NULL );
}

ParticleGraphic*		ParticleGraphicsFind(const char* szTextureName, eRenderFlags renderFlags, int layer )
{
ParticleGraphic*		pParticleGraphic = mspParticleGraphics;

	while( pParticleGraphic )
	{
		if ( ( stricmp( pParticleGraphic->GetTextureName(), szTextureName ) == 0 ) &&
			 ( pParticleGraphic->GetRenderFlags() == renderFlags ) &&
			 ( pParticleGraphic->GetLayer() == layer ) )
		{
			return( pParticleGraphic );
		}
		 
		pParticleGraphic = pParticleGraphic->GetNext();
	}

	return( NULL );
}

int		ParticleGraphicsCreateHandle( int hTex, float fGridScale, BOOL bUseRotation, eRenderFlags renderFlags, int layer )
{
ParticleGraphic*		pParticleGraphic = ParticleGraphicsFindTexHandle( hTex, renderFlags, layer );

	if ( pParticleGraphic )
	{
		return( pParticleGraphic->GetParticleGraphicID() );
	}

	pParticleGraphic = new ParticleGraphic;
	pParticleGraphic->InitFromTexHandle( hTex, fGridScale, bUseRotation, renderFlags, layer );
	pParticleGraphic->SetParticleGraphicID( msnNextParticleGraphicID );
	msnNextParticleGraphicID++;

	pParticleGraphic->SetNext( mspParticleGraphics );
	mspParticleGraphics = pParticleGraphic;

	return( pParticleGraphic->GetParticleGraphicID() );
	
}

uint32		ParticleGraphicsGetRenderFlags(int nParticleGraphicID)
{
	ParticleGraphic* pParticleGraphic = ParticleGraphicsFindFromID(nParticleGraphicID);

	if (pParticleGraphic)
	{
		return( pParticleGraphic->GetRenderFlags() );
	}
	return( kRenderFlag_Rotated );
}

int		ParticleGraphicsGetTextureHandle(int nParticleGraphicID)
{
	ParticleGraphic* pParticleGraphic = ParticleGraphicsFindFromID(nParticleGraphicID);

	if (pParticleGraphic)
	{
		return( pParticleGraphic->GetTextureHandle() ); 
	}
	return(NOTFOUND);
}

int		ParticleGraphicsCreate( const char* szTextureName, float fGridScale, BOOL bUseRotation, eRenderFlags renderFlags, int layer )
{
ParticleGraphic*		pParticleGraphic = ParticleGraphicsFind( szTextureName, renderFlags, layer );

	if ( pParticleGraphic )
	{
		return( pParticleGraphic->GetParticleGraphicID() );
	}

	pParticleGraphic = new ParticleGraphic;
	pParticleGraphic->Init( szTextureName, fGridScale, bUseRotation, renderFlags, layer );
	pParticleGraphic->SetParticleGraphicID( msnNextParticleGraphicID );
	msnNextParticleGraphicID++;

	pParticleGraphic->SetNext( mspParticleGraphics );
	mspParticleGraphics = pParticleGraphic;

	return( pParticleGraphic->GetParticleGraphicID() );
}

int		ParticleGraphicsGetNumActiveSpriteGroups()
{
	return( msnNumParticleGraphics );
}

SpriteGroup*		ParticleGraphicsGetSpriteGroup( int nParticleGraphicID )
{
ParticleGraphic*		pParticleGraphic = ParticleGraphicsFindFromID( nParticleGraphicID );
	
	if ( pParticleGraphic )
	{
		return( pParticleGraphic->GetSpriteGroup() );
	}
	return( NULL );
}

