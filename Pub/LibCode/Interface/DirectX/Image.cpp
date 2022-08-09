
#include "InterfaceInternalsDX.h"

#include <stdio.h>

#include <StandardDef.h>
#include <Interface.h>

#include "Jpeg/ShowJpeg.h"
#include "Image.h"

#define MAX_LOADED_IMAGES		64


#ifdef TUD9
//LPDIRECT3DSURFACE9			m_apImageSurfaces[ MAX_LOADED_IMAGES ];
IGRAPHICSSURFACE*				m_apImageSurfaces[ MAX_LOADED_IMAGES ];
#else
LPDIRECT3DSURFACE8			m_apImageSurfaces[ MAX_LOADED_IMAGES ];
#endif
int			GetFreeImageHandle( void )
{
int		nLoop;
	for( nLoop = 0; nLoop < MAX_LOADED_IMAGES; nLoop++ )
	{
		if ( m_apImageSurfaces[ nLoop ] == NULL )
		{
			return( nLoop );
		}
	}
	return( -1 );
}

INTERFACE_API void				InterfaceGetImageSize( IMAGEHANDLE nHandle, int* pnW, int* pnH )
{
	*pnW = 0;
	*pnH = 0;

	if ( ( nHandle >= 0 ) && ( nHandle < MAX_LOADED_IMAGES ) )
	{
		if ( m_apImageSurfaces[ nHandle ] )
		{
		D3DSURFACE_DESC		xDesc;
		IDirect3DSurface9*	pSurface;
			
			pSurface = m_apImageSurfaces[ nHandle ];
			if ( pSurface )
			{
				pSurface->GetDesc( &xDesc );
				*pnW = xDesc.Width;
				*pnH = xDesc.Height;
			}
		}
	}
}

INTERFACE_API IMAGEHANDLE		InterfaceLoadImageEx( const char* szFilename,int scaleToX, int scaleToY, unsigned int uFlags )
{
int		nHandle = GetFreeImageHandle();

	if ( nHandle != -1 )
	{
		m_apImageSurfaces[ nHandle ] = LoadJpegDirect( szFilename, scaleToX, scaleToY, (int)uFlags );
	}

	return( (IMAGEHANDLE)( nHandle ) );
}

INTERFACE_API IMAGEHANDLE		InterfaceLoadImage( const char* szFilename, unsigned int uFlags )
{
int		nHandle = GetFreeImageHandle();

	if ( nHandle != -1 )
	{
		if ( uFlags & 1 ) 
		{
			m_apImageSurfaces[ nHandle ] = LoadJpegDirect( szFilename, InterfaceGetWidth(), InterfaceGetHeight(), (int)uFlags );
		}
		else
		{
			m_apImageSurfaces[ nHandle ] = LoadJpegDirect( szFilename, 0, 0, (int)uFlags );
		}
	}

	return( (IMAGEHANDLE)( nHandle ) );
}

INTERFACE_API IMAGEHANDLE		InterfaceLoadImageFromArchive( const char* szFilename, unsigned int uFlags, int nArchive )
{
int		nHandle = GetFreeImageHandle();

	if ( nHandle != -1 )
	{
		if ( uFlags & 1 ) 
		{
			m_apImageSurfaces[ nHandle ] = LoadJpegDirectArchive( szFilename, InterfaceGetWidth(), InterfaceGetHeight(), (int)uFlags, nArchive );
		}
		else
		{
			m_apImageSurfaces[ nHandle ] = LoadJpegDirectArchive( szFilename, 0, 0, (int)uFlags, nArchive );
		}
	}

	return( (IMAGEHANDLE)( nHandle ) );
}




INTERFACE_API void				InterfaceDrawImage( int layer, IMAGEHANDLE nHandle, int X, int Y, int width, int height, unsigned int uFlags )
{
	if ( ( nHandle >= 0 ) && ( nHandle < MAX_LOADED_IMAGES ) )
	{
		if ( m_apImageSurfaces[ nHandle ] )
		{
			DrawJpegDirect( layer, m_apImageSurfaces[ nHandle ], X, Y, width, height, uFlags );
		}
	}
}



INTERFACE_API void				InterfaceReleaseImage( IMAGEHANDLE nHandle )
{
	if ( ( nHandle >= 0 ) && ( nHandle < MAX_LOADED_IMAGES ) )
	{
		if ( m_apImageSurfaces[ nHandle ] )
		{
			m_apImageSurfaces[ nHandle ]->Release();
			m_apImageSurfaces[ nHandle ] = NULL;
		}
	}

}



void		InterfaceImagesInit( void )
{
int		nLoop;

	for( nLoop = 0; nLoop < MAX_LOADED_IMAGES; nLoop++ )
	{
		m_apImageSurfaces[ nLoop ] = NULL;
	}

}


void		InterfaceImagesUpdate( void )
{



}

void		InterfaceImagesFree( void )
{
int		nLoop;

	for( nLoop = 0; nLoop < MAX_LOADED_IMAGES; nLoop++ )
	{
		if ( m_apImageSurfaces[ nLoop ] )
		{
			InterfaceReleaseImage( nLoop );
		}
	}
}