
#include "StandardDef.h"
#include "Engine.h"
#include "Interface.h"

#include "../Platform/Platform.h"

#include "LandscapeShader.h"
#include "SourceTextures.h"

int		mahSourceTextures[4] = { NOTFOUND };

const char*		maszSourceTextureFilenames[] = 
{
	"Data\\Textures\\Land\\gen\\defsnow.bmp",
	"Data\\Textures\\Land\\gen\\ZiogenRock1.jpg",
	"Data\\Textures\\Land\\gen\\rev-grass1f.jpg",
	"Data\\Textures\\Land\\gen\\ZiogenSand3.jpg",
};

int	SourceTexturesGetTextureHandle( int index )
{
	return( mahSourceTextures[index] );
}

void	SourceTexturesFree( void )
{
int		nLoop = 0;

	for ( nLoop = 0; nLoop < 4; nLoop++ )
	{
		EngineReleaseTexture( &mahSourceTextures[nLoop] );
	}
//	EngineReleaseTexture( &mhGrassNormal );
}

/***************************************************************************
 * Function    : LandscapeShaderLoadSourceTextures
 * 
 * Description : Load autogen source textures
 ***************************************************************************/
BOOL	SourceTexturesLoad( void )
{
int		nLoop = 0;
int		nLoadCounter = 0;

	for ( nLoop = 0; nLoop < 4; nLoop++ )
	{
		mahSourceTextures[nLoop] = EngineLoadTexture( maszSourceTextureFilenames[nLoop], 0 );

		LandscapeShaderSetSourceTexture( nLoop, mahSourceTextures[nLoop] );
	}

	BOOL	bNotComplete = TRUE;

	while( bNotComplete )
	{
		bNotComplete = FALSE;
		SysSleep( 1 );
		nLoadCounter++;
		EngineUpdate( FALSE );
		for ( nLoop = 0; nLoop < 4; nLoop++ )
		{
			if ( EngineTextureIsFullyLoaded(mahSourceTextures[nLoop]) == FALSE )
			{
				bNotComplete = TRUE;
				continue;
			}
		}
		if ( ( bNotComplete == TRUE ) &&
			 ( nLoadCounter > 100 ) )
		{
		int		nLineY = 40;

			EngineUpdate( TRUE );
			InterfaceNewFrame( 0 );

			InterfaceText( 0, 10, 10, "Loading landscape source textures..", 0xD0D0D0D0, 0 );

			for ( nLoop = 0; nLoop < 4; nLoop++ )
			{
				InterfaceText( 0, 100, nLineY,  maszSourceTextureFilenames[nLoop], 0xd0c0c0c0, 0 );

				if ( EngineTextureIsFullyLoaded(mahSourceTextures[nLoop]) == FALSE )
				{
					InterfaceText( 0, 40, nLineY, "false", 0xd0c06020, 0 );
				}
				else
				{
					InterfaceText( 0, 40, nLineY, "OK", 0xd020e060, 0 );
				}
				nLineY += 20;
			}

			InterfaceBeginRender();
			InterfaceDraw();
			InterfaceEndRender();
			InterfacePresent();
			nLoadCounter = 0;

			if ( PlatformUpdateSystemEx() == FALSE )
			{
				return( FALSE );
			}
		}
	}
	return( TRUE );
	
//	mhGrassNormal = EngineLoadTexture( "Data\\Textures\\Land\\gen\\GrassNormal.jpg", NO_MIPMAPPING );
}
