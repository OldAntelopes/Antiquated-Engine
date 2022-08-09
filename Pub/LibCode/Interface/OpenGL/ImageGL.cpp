
#include <stdio.h>

#include <StandardDef.h>
#include <Interface.h>

#include "ImageGL.h"

#define MAX_LOADED_IMAGES		64

typedef struct
{
	int*	pnImage;

} IMAGE_STRUCTURE;

IMAGE_STRUCTURE*			m_apImageSurfaces[ MAX_LOADED_IMAGES ];

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
	}
}

INTERFACE_API IMAGEHANDLE		InterfaceLoadImageEx( const char* szFilename,int scaleToX, int scaleToY, unsigned int uFlags )
{


	return( -1 );
}


INTERFACE_API IMAGEHANDLE		InterfaceLoadImage( const char* szFilename, unsigned int uFlags )
{
int		nHandle = GetFreeImageHandle();

	if ( nHandle != -1 )
	{
		if ( uFlags & 1 ) 
		{
//			m_apImageSurfaces[ nHandle ] = LoadJpegDirect( szFilename, InterfaceGetWidth(), InterfaceGetHeight(), (int)uFlags );
		}
		else
		{
//			m_apImageSurfaces[ nHandle ] = LoadJpegDirect( szFilename, 0, 0, (int)uFlags );
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
//			DrawJpegDirect( layer, m_apImageSurfaces[ nHandle ], X, Y, width, height, uFlags );
		}
	}
}



INTERFACE_API void				InterfaceReleaseImage( IMAGEHANDLE nHandle )
{
	if ( ( nHandle >= 0 ) && ( nHandle < MAX_LOADED_IMAGES ) )
	{
		if ( m_apImageSurfaces[ nHandle ] )
		{
//			m_apImageSurfaces[ nHandle ]->Release();
//			m_apImageSurfaces[ nHandle ] = NULL;
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
//			InterfaceReleaseImage( nLoop );
		}
	}
}