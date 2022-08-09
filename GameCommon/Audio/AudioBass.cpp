
#include "StdWinInclude.h"
#include "StandardDef.h"

#include "Bass.h"

#include "../Platform/Platform.h"
#include "Audio.h"

void		AudioSoundLoadSingle( const char* szSoundName, const char* pcSoundFilename )
{
	// TODO

}

void		AudioSoundPlay( const char* szSoundName, float fVolume )
{
	// TODO

}

void		AudioInit( void )
{
#ifdef BASS_INCLUDED
HWND	hwndMain = (HWND)PlatformGetWindowHandle();

	if ( BASS_Init(-1,44100,0,hwndMain,NULL) == FALSE )
	{
		SysDebugPrint( "Bass Init failed" );

	}
#endif
}

void		AudioUpdate( float delta )
{

}


void		AudioShutdown( void )
{

}
