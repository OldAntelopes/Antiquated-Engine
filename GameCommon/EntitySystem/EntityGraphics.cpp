
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "EntityGraphics.h"

class EntityGraphic
{
public:
	EntityGraphic();
	~EntityGraphic();

	void			Init( const char* szModelName, const char* szTextureName, int nInstanceNum );

	const char*		GetModelName( void ) { return( mszModelName ); }
	const char*		GetTextureName( void ) { return( mszTextureName ); }
	int				GetInstanceNum( void ) { return( mnInstanceNum ); }

	int				GetModelHandle( void ) { return( mhModel ); }
	int				GetTextureHandle( void ) { return( mhTexture ); }

	void				SetEntityGraphicID( int nID ) { mnEntityGraphicID = nID; }
	int					GetEntityGraphicID( void ) { return( mnEntityGraphicID ); }

	EntityGraphic*		GetNext( void ) { return( mpNext ); }
	void				SetNext( EntityGraphic* pNext ) { mpNext = pNext; }
private:
	
	int					mhModel;
	int					mhTexture;

	char*				mszModelName;
	char*				mszTextureName;
	int					mnInstanceNum;

	int					mnEntityGraphicID;

	EntityGraphic*		mpNext;

};

EntityGraphic*		mspEntityGraphics = NULL;
int					msnNextEntityGraphicID = 0x100;


EntityGraphic::EntityGraphic()
{
	mszModelName = NULL;
	mszTextureName = NULL;
	mnInstanceNum = 0;
	mhModel = NOTFOUND;
	mhTexture = NOTFOUND;
	mpNext = NULL;
}

EntityGraphic::~EntityGraphic()
{
	if ( mszModelName )
	{
		free( mszModelName );
	}
	if ( mszTextureName )
	{
		free( mszTextureName );
	}
	if ( mhModel != NOTFOUND )
	{
		ModelFree( mhModel );
		mhModel = NOTFOUND;
	}
	if ( mhTexture != NOTFOUND )
	{
		EngineReleaseTexture( &mhTexture );
	}
}

void		EntityGraphic::Init( const char* szModelName, const char* szTextureName, int nInstanceNum )
{
	mszModelName = (char*)( SystemMalloc( strlen( szModelName ) + 1 ) );
	strcpy( mszModelName, szModelName );

	if ( szTextureName )
	{
		mszTextureName = (char*)( SystemMalloc( strlen( szTextureName ) + 1 ) );
		strcpy( mszTextureName, szTextureName );
	}
	mnInstanceNum = nInstanceNum;

	mhModel = ModelLoad( szModelName, 0, 1.0f );
	mhTexture = EngineLoadTexture( szTextureName, 0, 0 );
}


EntityGraphic*		EntityGraphicsFindFromID( int nEntityGraphicID )
{
EntityGraphic*		pEntityGraphic = mspEntityGraphics;

	while( pEntityGraphic )
	{
		if ( pEntityGraphic->GetEntityGraphicID() == nEntityGraphicID )
		{
			return( pEntityGraphic );
		}
		pEntityGraphic = pEntityGraphic->GetNext();
	}
	return( NULL );
}

EntityGraphic*		EntityGraphicsFind( const char* szModelName, const char* szTextureName, int nInstanceNum )
{
EntityGraphic*		pEntityGraphic = mspEntityGraphics;

	while( pEntityGraphic )
	{
		if ( ( nInstanceNum == pEntityGraphic->GetInstanceNum() ) &&
			 ( stricmp( pEntityGraphic->GetModelName(), szModelName ) == 0 ) &&
			 ( stricmp( pEntityGraphic->GetTextureName(), szTextureName ) == 0 ) )
		{
			return( pEntityGraphic );
		}
		 
		pEntityGraphic = pEntityGraphic->GetNext();
	}

	return( NULL );
}

int		EntityGraphicsCreate( const char* szModelName, const char* szTextureName, int nInstanceNum )
{
EntityGraphic*		pEntityGraphic = EntityGraphicsFind( szModelName, szTextureName, nInstanceNum );

	if ( pEntityGraphic )
	{
		return( pEntityGraphic->GetEntityGraphicID() );
	}

	pEntityGraphic = new EntityGraphic;
	pEntityGraphic->Init( szModelName, szTextureName, nInstanceNum );
	pEntityGraphic->SetEntityGraphicID( msnNextEntityGraphicID );
	msnNextEntityGraphicID++;

	pEntityGraphic->SetNext( mspEntityGraphics );
	mspEntityGraphics = pEntityGraphic;

	return( pEntityGraphic->GetEntityGraphicID() );
}

int		EntityGraphicsGetModelHandle( int nEntityGraphicNum )
{
EntityGraphic*		pEntityGraphic = EntityGraphicsFindFromID( nEntityGraphicNum );
	
	if ( pEntityGraphic )
	{
		return( pEntityGraphic->GetModelHandle() );
	}
	return( NOTFOUND );
}

int		EntityGraphicsGetTextureHandle( int nEntityGraphicNum )
{
EntityGraphic*		pEntityGraphic = EntityGraphicsFindFromID( nEntityGraphicNum );
	
	if ( pEntityGraphic )
	{
		return( pEntityGraphic->GetTextureHandle() );
	}
	return( NOTFOUND );
}

void		EntityGraphicsDeleteAll( void )
{
EntityGraphic*		pEntityGraphic = mspEntityGraphics;
EntityGraphic*		pNext;

	while( pEntityGraphic )
	{
		pNext = pEntityGraphic->GetNext();
		delete pEntityGraphic;
		pEntityGraphic = pNext;
	}
	mspEntityGraphics = NULL;
}
