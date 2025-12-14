
#include <stdio.h>

#include <StandardDef.h>
#include <Interface.h>

#include "InterfaceTextureList.h"
#include "InterfaceUtil.h"

BOOL	mboZBufferLockable = TRUE;

INTERFACE_API BOOL InterfaceIsZBufferLockable( void )
{
	return( mboZBufferLockable);
}

INTERFACE_API void InterfaceSetZBufferLockable( BOOL boFlag )
{
	mboZBufferLockable = boFlag;
}


uint32		ColourPack( int R, int G, int B, int A )
{
uint32	ulCol;

	ulCol = (R << 16) | (G << 8 ) | B | (A << 24);
	return( ulCol );
}

void		ColourUnpack( uint32 ulCol, int* pulRed, int* pulGreen, int* pulBlue, int* pulAlpha )
{
	*pulRed = ( ulCol >> 16 ) & 0xFF;
	*pulGreen = ( ulCol >> 8 ) & 0xFF;
	*pulBlue = ulCol & 0xFF;
	*pulAlpha = ( ulCol >> 24 ) & 0xFF;

}

uint32		InterfaceColourLightenLinear( uint32 ulCol, uint32 ulLightenAmount )
{
int		nR, nG, nB, nA;
uint32		ulOut;

	ColourUnpack( ulCol, &nR, &nG, &nB, &nA );
	nR += ulLightenAmount;
	if ( nR > 255 ) nR = 255;
	nG += ulLightenAmount;
	if ( nG > 255 ) nG = 255;
	nB += ulLightenAmount;
	if ( nB > 255 ) nB = 255;

	ulOut = ColourPack( nR, nG, nB, nA );
	return( ulOut );
}

uint32		InterfaceColourDarkenLinear( uint32 ulCol, uint32 ulDarkenAmount )
{
int		nR, nG, nB, nA;
uint32		ulOut;

	ColourUnpack( ulCol, &nR, &nG, &nB, &nA );
	nR -= ulDarkenAmount;
	if ( nR < 0 ) nR = 0;
	nG -= ulDarkenAmount;
	if ( nG < 0 ) nG = 0;
	nB -= ulDarkenAmount;
	if ( nB < 0 ) nB = 0;

	ulOut = ColourPack( nR, nG, nB, nA );
	return( ulOut );
}

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

