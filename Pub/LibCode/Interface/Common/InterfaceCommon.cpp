
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

int		InterfaceTextBox( int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth, BOOL bLeftAlign )
{
char*	pcEndOfLine = (char*)szString;
int		nLineSep = 13;
int		nBaseY = nY;
int		nStringWidth;

	// TODO - Adjust Y separation on font and scale
	switch( font )
	{
	case 1:
		nLineSep = 19;
		break;
	case 2:
		nLineSep = 15;
		break;
	default:
		break;
	}

	nStringWidth = GetStringWidth( szString, font );

	// If whole text fits on a single line
	if ( nStringWidth < nMaxWidth )
	{
		if ( bLeftAlign )
		{
			InterfaceText( nLayer, nX, nY, szString, ulCol, font );
		}
		else
		{
			InterfaceTextCenter( nLayer, nX, nX + nMaxWidth, nY, szString, ulCol, font );
		}
		nY += nLineSep;
	}
	else
	{
		while ( pcEndOfLine != NULL )
		{
			pcEndOfLine = InterfaceTextLimitWidth( nLayer, nX, nY, (char*)pcEndOfLine, ulCol, font, nMaxWidth );
			nY += nLineSep;
		}
	}
	return( nY - nBaseY );
}	
