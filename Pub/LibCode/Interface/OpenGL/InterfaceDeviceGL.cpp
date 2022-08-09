
#include <windows.h>			// For OpenGL
#include <stdio.h>
#include <gl/gl.h>

#include <StandardDef.h>
#include <Interface.h>


#include "../Common/InterfaceDevice.h"


void		OnSetInitialSize( BOOL boFullScreen, int nFullScreenSizeX, int nFullScreenSizeY , BOOL boSmallFlag )
{
	glViewport( 0,0, InterfaceGetWidth(), InterfaceGetHeight() );

}



void		OnSetWindowSize( BOOL boFullScreen, int nWidth, int nHeight )
{
	glViewport( 0,0, nWidth, nHeight );

}
