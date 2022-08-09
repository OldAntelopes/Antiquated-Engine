
#include <stdio.h>

#include <StandardDef.h>		// This specifies a minimal set of 'standard' defines like BOOL
#include <Interface.h>			// The 2d graphics interface library

#include "OSD.h"

IMAGEHANDLE		m_OSDBackgroundImage = NOTFOUND;

int				m_OSDExampleTexture = NOTFOUND;
int				m_OSDExampleTexture2 = NOTFOUND;

float			m_BounceAngle = 0.0f;

//-------------------------------------------------------------------------
// Function    : OSDDrawOptions
// Description : TBD
//-------------------------------------------------------------------------
void		OSDDrawOptions( int x, int y, int w, int h )
{
	InterfaceRect( 1, x, y, w, 45, 0x10000000 );

	InterfaceSetFontFlags( FONT_FLAG_DROP_SHADOW|FONT_FLAG_GIANT );
	
	InterfaceTextCenter( 1, x, x + w, y, "Options", 0xE0E0D040, 1 );

	InterfaceSetFontFlags( 0 );
}


//-------------------------------------------------------------------------
// Function    : OSDDrawExamples
// Description : TBD
//-------------------------------------------------------------------------
void		OSDDrawExamples( int x, int y, int w, int h )
{
int		nOverlay = InterfaceCreateNewTexturedOverlay( 1, m_OSDExampleTexture );
int		nOverlay2 = InterfaceCreateNewTexturedOverlay( 1, m_OSDExampleTexture2 );

	InterfaceText( 1, x + 30, y, "Example textured rect:", 0xE0C0C0C0, 1 );
	// Draw a bouncing sphere
	InterfaceTexturedRect( nOverlay, x + 60, y + 30, 128, 128, 0xFFFFFFFF, 0.0f, 0.0f, 1.0f, 1.0f );

	InterfaceText( 1, x + 300, y, "Example PNG (50% trans):", 0xE0C0C0C0, 1 );

	InterfaceTexturedRect( nOverlay2, x + 330, y + 30, 240, 240, 0x80FFFFFF, 0.0f, 0.0f, 1.0f, 1.0f );

//	InterfaceSprite( nOverlay, x, y, 1.0f, 0, 0xFFFFFFFF, 0.0f, 0.0f );
}

//-------------------------------------------------------------------------
// Function    : OSDDrawBackground
// Description : Example of using a couple of the basic Interface rendering calls
//               to draw some background graphics
//-------------------------------------------------------------------------
void		OSDDrawBackground( void )
{
int		width = InterfaceGetWidth();		// Get the width and height of the interface
int		height = InterfaceGetHeight();

	// Lets draw a shaded box over the entire background to give it a bit of colour
	InterfaceShadedRect( 0, 0, 0, width, height, 0xFF300000, 0xFF300000, 0xFF702000, 0xFF702000 );  // dark red at the top, a slightly less dark red at the bottom

	// And draw a jpeg background image over the top of that
//	InterfaceDrawImage( 0, m_OSDBackgroundImage, 16, 10, 0,0, 0 );
	//   (Note that the InterfaceTexturedRect (etc) functions provide a more optimal way of rendering imagery like this, but
	//    in those functions the image data is converted to a 'texture' first, and hence may end up being resized/filtered etc. The
	//     InterfaceDrawImage function is there for when you need a 'clean', unscaled image is required)

}

//-------------------------------------------------------------------------
// Function    : OSDShowFPS
// Description : Example of text rendering to show a useful on-screen indicator of FPS
//-------------------------------------------------------------------------
void		OSDShowFPS( void )
{
int		x = InterfaceGetWidth() - 10;		
int		y = InterfaceGetHeight() - 20;
char	szString[256];

	// Draw the FPS count at the bottom right of the screen
	sprintf( szString, "FPS : %.1f", InterfaceGetFPS() );
	InterfaceTextRight( 1, x, y, szString, 0xE0F0F0F0, 0 );
}


//-------------------------------------------------------------------------
// Function    : OSDRender
// Description : This is the OSD's Render function called every frame from the main update loop
//               It triggers calls to the other functions to draw various components
//-------------------------------------------------------------------------
void		OSDRender( void )
{
	// Draw some stuff
	OSDDrawBackground();

	// Draw some examples
	OSDDrawExamples( 0, 30, 800, 270 );
 
	// Draw some more stuff
	OSDDrawOptions( 0, 300, 800, 270 );

	// Show the FPS count on the bottom right of the screen
	OSDShowFPS();
}


//-------------------------------------------------------------------------
// Function    : OSDInitialise
// Description : Called once at startup from the main initialise function
//               Loads up a jpg 'Image'
//-------------------------------------------------------------------------
void		OSDInitialise( void )
{
	// Load a jpeg image for the background
	m_OSDBackgroundImage = InterfaceLoadImage( "Data\\Samples\\snow1.jpg", 0 );

	// Load a texture
	m_OSDExampleTexture = InterfaceLoadTexture( "Data\\Samples\\ballsprite.bmp", 0 );
	m_OSDExampleTexture2 = InterfaceLoadTexture( "Data\\Samples\\ballsprite2.png", 0 );
}



//-------------------------------------------------------------------------
// Function    : OSDFree
// Description : Called when we're exiting the program. 
//				 Frees up any loaded graphics and resources
//-------------------------------------------------------------------------
void		OSDFree( void )
{
	InterfaceReleaseImage( m_OSDBackgroundImage );
	m_OSDBackgroundImage = NOTFOUND;
}
