
#include "../Marmalade/InterfaceInternalsMarmalade.h"

#include <StandardDef.h>
#include <Interface.h>

#include "InterfaceTextureList.h"

#define		MAX_INTERNAL_TEXTURES_LOADED		256


typedef struct
{
	LPGRAPHICSTEXTURE		pTexture;
	char					acFilename[128];
	ulong					ulLastTouched;
	short					wRefCount;
	short					wInMorgue;
	int						nPitch;

} INTERNAL_TEXTURES;

INTERNAL_TEXTURES	maxInternalTextures[MAX_INTERNAL_TEXTURES_LOADED];

int		msnTextureListInMorgue = 0;

int			InterfaceFindTexture( const char* szFilename )
{
int		nLoop = 0;

	do
	{
		if ( maxInternalTextures[ nLoop ].pTexture != NULL )
		{
			if ( strnicmp( maxInternalTextures[ nLoop ].acFilename, szFilename, 127 ) == 0 )
			{
				return( nLoop );
			}
		}
		nLoop++;
	} while ( nLoop < MAX_INTERNAL_TEXTURES_LOADED );
	return( NOTFOUND );
}

int		InterfaceGetNewInternalTextureHandle( void )
{
int		nLoop = 0;
	
	do
	{
		if ( maxInternalTextures[ nLoop ].pTexture == NULL )
		{
			return( nLoop );
		}
		nLoop++;
	} while ( nLoop < MAX_INTERNAL_TEXTURES_LOADED );

	return( NOTFOUND );
}


int	InterfaceCreateBlankTexture( int nWidth, int nHeight, int Mode)
{
int		nHandle;

	nHandle = InterfaceGetNewInternalTextureHandle();

	if ( nHandle != NOTFOUND )
	{
		maxInternalTextures[ nHandle ].pTexture = InterfaceGetBlankPlatformTexture( nWidth, nHeight, Mode, &maxInternalTextures[ nHandle ].nPitch );
		maxInternalTextures[ nHandle ].wRefCount = 1;
		strcpy( maxInternalTextures[ nHandle ].acFilename, "gentex" );
	}
	return( nHandle );
}


int	InterfaceGetTextureInternal( const char* szFilename, int nFlags, int hArchive )
{
int		nHandle;

	// Look for this filename if its already loaded
	nHandle = InterfaceFindTexture( szFilename );
	if ( nHandle >= 0 )
	{
		// If found, add a reference to this texture
		maxInternalTextures[ nHandle ].wRefCount++;
		return( nHandle );
	}

	nHandle = InterfaceGetNewInternalTextureHandle();

	if ( nHandle != NOTFOUND )
	{
		maxInternalTextures[ nHandle ].pTexture = InterfaceGetPlatformTexture( szFilename, nFlags, hArchive, &maxInternalTextures[ nHandle ].nPitch );
		maxInternalTextures[ nHandle ].wRefCount = 1;
//		maxInternalTextures[ nHandle ].ulLastTouched;
		return( nHandle );
	}
	return( NOTFOUND );
}

INTERFACE_API int	InterfaceGetTexture( const char* szFilename, int nFlags )
{
	return( InterfaceGetTextureInternal( szFilename, nFlags, -1 ) );
}

int			InterfaceTextureListGetPitch( int nTextureHandle )
{
	if ( nTextureHandle != NOTFOUND )
	{
		return( maxInternalTextures[ nTextureHandle ].nPitch );
	}
	return( 0 );
}

void*		InterfaceTextureListGetPlatformTexture( int nTextureHandle )
{
	if ( nTextureHandle != NOTFOUND )
	{
		return( maxInternalTextures[ nTextureHandle ].pTexture );
	}
	return( NULL );
}

void	InterfaceTextureListNewFrame( void )
{
	if ( msnTextureListInMorgue > 0 )
	{
	int		nLoop;

		for( nLoop = 0; nLoop < MAX_INTERNAL_TEXTURES_LOADED; nLoop++ )
		{
			if ( maxInternalTextures[nLoop].wInMorgue > 0 )
			{
				maxInternalTextures[nLoop].wInMorgue--;

				if ( maxInternalTextures[nLoop].wInMorgue == 0 )
				{
					InterfaceReleasePlatformTexture( nLoop, maxInternalTextures[ nLoop ].pTexture );
					maxInternalTextures[ nLoop ].pTexture = NULL;
					maxInternalTextures[ nLoop ].acFilename[0] = 0;
					maxInternalTextures[ nLoop ].ulLastTouched = 0;
					maxInternalTextures[ nLoop ].wRefCount = 0;
					maxInternalTextures[ nLoop ].wInMorgue = 0;
					msnTextureListInMorgue--;

					if ( msnTextureListInMorgue == 0 )
					{
						return;
					}
				}

			}
		}
	}
}


void		InterfaceTextureListInit( void )
{
	ZeroMemory( maxInternalTextures, sizeof( INTERNAL_TEXTURES ) * MAX_INTERNAL_TEXTURES_LOADED );
}

void		InterfaceTextureListFree( void )
{
int		nLoop = 0;

	do
	{
		if ( maxInternalTextures[ nLoop ].pTexture != NULL )
		{
			InterfaceReleasePlatformTexture( nLoop, maxInternalTextures[ nLoop ].pTexture );
		}
		nLoop++;

	} while ( nLoop < MAX_INTERNAL_TEXTURES_LOADED );

	ZeroMemory( maxInternalTextures, sizeof( INTERNAL_TEXTURES ) * MAX_INTERNAL_TEXTURES_LOADED );
}

INTERFACE_API void InterfaceReleaseTexture( int nTextureHandle )
{
	if ( nTextureHandle != NOTFOUND )
	{
		nTextureHandle %= MAX_INTERNAL_TEXTURES_LOADED;
		if ( maxInternalTextures[ nTextureHandle ].pTexture != NULL )
		{ 
			if ( maxInternalTextures[ nTextureHandle ].wRefCount > 1 )
			{
				maxInternalTextures[ nTextureHandle ].wRefCount--;
			}
			else
			{
#ifdef IW_SDK
				maxInternalTextures[ nTextureHandle ].wInMorgue = 2;
				msnTextureListInMorgue++;
//				InterfaceReleasePlatformTexture( maxInternalTextures[ nTextureHandle ].pTexture );
#endif

#ifdef TUD9
				maxInternalTextures[ nTextureHandle ].pTexture->Release();

				maxInternalTextures[ nTextureHandle ].pTexture = NULL;
				maxInternalTextures[ nTextureHandle ].acFilename[0] = 0;
				maxInternalTextures[ nTextureHandle ].ulLastTouched = 0;
				maxInternalTextures[ nTextureHandle ].wRefCount = 0;
				maxInternalTextures[ nTextureHandle ].wInMorgue = 0;
#endif
			}
		}
	}
}
