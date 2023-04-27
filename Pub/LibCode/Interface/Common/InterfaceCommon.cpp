
#include <stdio.h>

#include <StandardDef.h>
#include <Interface.h>

#include "../Win32/Interface-Win32.h"
#include "InterfaceCommon.h"
#include "InterfaceUtil.h"
#include "InterfaceDevice.h"

int		mnDesiredWidth = 1024;
int		mnDesiredHeight = 768;
BOOL	mboInterfaceVRMode = FALSE;

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

INTERFACE_API void	InterfaceSetVRMode( BOOL bVRModeActive )
{
	mboInterfaceVRMode  = bVRModeActive;
} 

BOOL		InterfaceIsVRMode( void )
{
	return( mboInterfaceVRMode );
}

INTERFACE_API void InterfaceGetInitialScreenSize( int* pnWidth, int* pnHeight )
{
	*(pnWidth) = mnDesiredWidth;
	*(pnHeight) = mnDesiredHeight;
}

BOOL					InterfaceIsOversized( void )
{
	if ( ( InterfaceGetWidth() > 1024 ) &&
		 ( InterfaceGetHeight() > 1024 ) )
	{
		return( TRUE );
	}
	return( FALSE );
}

INTERFACE_API void InterfaceSetInitialSize( BOOL boFullScreen, int nFullScreenSizeX, int nFullScreenSizeY , BOOL boSmallFlag )
{
//	mcd3dUtilApp.m_bWindowed = !boFullScreen;
	mnDesiredWidth = nFullScreenSizeX;
	mnDesiredHeight = nFullScreenSizeY;

	if ( !boFullScreen )
	{
#ifdef WIN32
		InterfaceWin32SetInitialWindowSize( nFullScreenSizeX, nFullScreenSizeY );
#endif
	}

#ifdef WIN32
	OnSetInitialSize( boFullScreen, nFullScreenSizeX, nFullScreenSizeY, boSmallFlag );
#endif

//	mboSmall = boSmallFlag;	// todo
}

