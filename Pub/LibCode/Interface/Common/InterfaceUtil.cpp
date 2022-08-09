
#include <stdio.h>

#include <StandardDef.h>
#include <Interface.h>

#include "InterfaceTextureList.h"
#include "InterfaceUtil.h"


void		InterfaceUnpackCol( unsigned int ulARGBCol, float* pfRed, float* pfGreen, float* pfBlue, float* pfAlpha )
{
	*pfAlpha = (float)( ulARGBCol >> 24 ) / 255.0f;
	*pfRed = (float)( (ulARGBCol >> 16 ) & 0xff ) / 255.0f;
	*pfGreen = (float)( (ulARGBCol >> 8 ) & 0xff ) / 255.0f;
	*pfBlue = (float)( ulARGBCol & 0xff ) / 255.0f;
}


INTERFACE_API int	InterfaceLoadTexture( const char* szFilename, int nFlags )
{
	return( InterfaceGetTexture( szFilename, nFlags ) );
}

INTERFACE_API int	InterfaceLoadTextureFromArchive( const char* szFilename, int nFlags, int nArchiveHandle )
{
	return( InterfaceGetTextureInternal( szFilename, nFlags, nArchiveHandle ) );
}

