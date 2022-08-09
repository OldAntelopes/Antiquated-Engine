
#include "StandardDef.h"
#include "Interface.h"

#include "StaticBackground.h"

 

void	StaticBackground::OnInitialise( void )
{
	if ( mhBackgroundTexture == NOTFOUND )
	{
		mhBackgroundTexture = InterfaceLoadTexture( "Data\\UI\\Backgrounds\\Background2a.png", 0 );
//		mhBackgroundTexture = InterfaceLoadTexture( "Data\\UI\\Backgrounds\\Background5.png", 0 );
	}
}

void	StaticBackground::OnDisplay( float fGlobalAlpha )
{
int		nBackgroundOverlay;

	nBackgroundOverlay = InterfaceCreateNewTexturedOverlay( 0, mhBackgroundTexture );
	InterfaceTexturedRect( nBackgroundOverlay, 0, 0, InterfaceGetWidth(), InterfaceGetHeight(), 0xFFE0E0E0, 0.0f, 0.0f, 1.0f, 1.0f );
}

void	StaticBackground::OnShutdown( void )
{
	InterfaceReleaseTexture( mhBackgroundTexture );
	mhBackgroundTexture = NOTFOUND;

}
