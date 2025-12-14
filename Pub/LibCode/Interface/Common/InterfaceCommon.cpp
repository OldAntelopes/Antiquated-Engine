
#include <stdio.h>
#include <math.h>
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

