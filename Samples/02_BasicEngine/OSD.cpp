
#include <stdio.h>

#include <StandardDef.h>		// This specifies a minimal set of 'standard' defines like BOOL
#include <Interface.h>			// The 2d graphics interface library

#include "OSD.h"


//-------------------------------------------------------------------------
// Function    : OSDDrawOptions
// Description : 
//-------------------------------------------------------------------------
void		OSDDrawOptions( void )
{



}




//-------------------------------------------------------------------------
// Function    : OSDShowFPS
// Description : 
//-------------------------------------------------------------------------
void		OSDShowFPS( void )
{
int		x = InterfaceGetWidth() - 10;		
int		y = InterfaceGetHeight() - 20;
char	szString[256];

	// Draw the FPS count at the bottom right of the screen
	sprintf( szString, "FPS : %.1f", InterfaceGetFPS() );
	InterfaceTextRight( 0, x, y, szString, 0xE0F0F0F0, 0 );
}


//-------------------------------------------------------------------------
// Function    : OSDRender
// Description : 
//-------------------------------------------------------------------------
void		OSDRender( void )
{
	// Draw some more stuff
	OSDDrawOptions();

	// Show the FPS count on the bottom right of the screen
	OSDShowFPS();
}



//-------------------------------------------------------------------------
// Function    : OSDInitialise
// Description : TBD
//-------------------------------------------------------------------------
void		OSDInitialise( void )
{

}



//-------------------------------------------------------------------------
// Function    : OSDFree
// Description : TBD
//-------------------------------------------------------------------------
void		OSDFree( void )
{

}
