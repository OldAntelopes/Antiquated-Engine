
#include "StandardDef.h"
#include "Interface.h"
#include "InterfaceOptions.h"

int		mnOptionTextureFiltering = TRUE;
int		mnOptionBackBuffer = TRUE;
int		mnOptionVsync = TRUE;
int		mnOptionOldStartup = TRUE;
int		mnOptionMinimumSurfaceRes = TRUE;
int		mnOptionFogMode = 0;
int		mnFullscreenAntialias = 0;


/***************************************************************************
 * Function    : InterfaceSetOption
 ***************************************************************************/
INTERFACE_API void InterfaceSetOption( int nOptionNum, int nValue )
{
	if ( nOptionNum >= MAX_OPTIONS )
	{
		return;
	}

	switch( nOptionNum )
	{
	case TEXTURE_FILTERING:
		mnOptionTextureFiltering = nValue;
		break;
	case BACK_BUFFER:
		mnOptionBackBuffer = nValue;
		break;
	case VSYNC:
		mnOptionVsync = nValue;
		break;
	case OLD_STARTUP:
		mnOptionOldStartup = nValue;
		break;
	case MINIMUM_SURFACE_RES:
		mnOptionMinimumSurfaceRes = nValue;
		break;
	case FOG_MODE:
		mnOptionFogMode = nValue;
		break;
	case FSAA:
		mnFullscreenAntialias = nValue;
		break;
	}

}


/***************************************************************************
 * Function    : InterfaceGetOption
 ***************************************************************************/
INTERFACE_API int InterfaceGetOption( int nOptionNum )
{
	switch( nOptionNum )
	{
	case TEXTURE_FILTERING:
		return( mnOptionTextureFiltering );
		break;
	case BACK_BUFFER:
		return( mnOptionBackBuffer );
		break;
	case VSYNC:
		return( mnOptionVsync );
		break;
	case OLD_STARTUP:
		return( mnOptionOldStartup );
		break;
	case MINIMUM_SURFACE_RES:
		return( mnOptionMinimumSurfaceRes );
		break;
	case FOG_MODE:
		return( mnOptionFogMode );
		break;
	case FSAA:
		return( mnFullscreenAntialias );
		break;
	}

	return( 0 );
}
