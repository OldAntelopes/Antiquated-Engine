
#include <stdio.h>

#include "StdWinInclude.h"
#include "StandardDef.h"

#include "Bass.h"
#include "../LibCode/CodeUtil/Archive.h"

#include "Audio.h"

char	mszMusicFilename[256] = "";
int		mnMusicArchiveHandle = NOTFOUND;
int		mnMusicArchiveFileHandle = NOTFOUND;
HSTREAM		mhBassStream = 0;
BOOL		mboMP3Active = FALSE;

void MusicTrackFinished( void )
{
//	FreeMusic(); 
}

DWORD CALLBACK	PlayMusicFromArchivefileProc( DWORD action, DWORD param1, DWORD param2, DWORD user )
{
Archive*		pArchive = ArchiveGetFromHandle( mnMusicArchiveHandle );
int		nRet = 0;

	if ( pArchive )
	{
		switch( action )
		{
		case BASS_FILE_CLOSE:
			if ( mnMusicArchiveFileHandle != NOTFOUND )
			{
				pArchive->CloseFile( mnMusicArchiveFileHandle );
				mnMusicArchiveFileHandle = NOTFOUND;
			}
			return 0;
			break;
		case BASS_FILE_READ:
			nRet = pArchive->ReadFile( mnMusicArchiveFileHandle, (byte*)param2, param1 );
			break;
		case BASS_FILE_LEN:
			nRet = pArchive->GetFileSize( mnMusicArchiveFileHandle );
			break;
		case BASS_FILE_SEEK:
			nRet = pArchive->SeekFile( mnMusicArchiveFileHandle, param1, 0 );
			break;
		}
	}
	return( nRet );
}



void AudioPlayMusic( const char* pcFileName, int nFullFileLength, int nArchiveHandle )
{
char	acString[512];
int		nFlags = 0;		//	nFlags = BASS_SAMPLE_MONO /BASS_STREAM_BLOCK;

	strcpy( mszMusicFilename, pcFileName );
	mnMusicArchiveHandle = nArchiveHandle;

	if ( mnMusicArchiveHandle > 0 )
	{
	Archive*		pArchive = ArchiveGetFromHandle( mnMusicArchiveHandle );

		mnMusicArchiveFileHandle = pArchive->OpenFile( pcFileName );

		if ( mnMusicArchiveFileHandle > 0 )
		{
			mhBassStream = BASS_StreamCreateFileUser( FALSE, nFlags, &PlayMusicFromArchivefileProc, 0 );
		}
	}
	else
	{
		mhBassStream = BASS_StreamCreateFile(FALSE,pcFileName,0,0,nFlags);
	}

	if ( !mhBassStream )
	{
		SysDebugPrint( "Error preparing mp3 music file - %s", pcFileName );
	}
	else
	{
		BASS_Start();
		BASS_SetConfig( BASS_CONFIG_GVOL_STREAM, 100 ); // mnMusicVolume );
		BASS_SetConfig( BASS_CONFIG_GVOL_MUSIC, 100 ); // mnMusicVolume );

		if (!BASS_ChannelPlay(mhBassStream,FALSE))
		{
			SysDebugPrint( "Error code %d playing music file - %s",BASS_ErrorGetCode(), pcFileName );
			MusicTrackFinished();
		}
		else
		{
			SysUserPrint( 0, "Playing - %s", pcFileName );
			mboMP3Active = TRUE;
		}
	}
}
