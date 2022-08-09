
#include "StandardDef.h"

#include <s3e.h>
#include <s3eSound.h>

#include "Audio.h"

class MarmaladeSound
{
public:
	char*		mszSoundName;
	ushort*		mpuwSoundData;
	int			mnSoundFileSize;

	MarmaladeSound*		mpNext;
};

MarmaladeSound*		mpMarmaladeSoundList = NULL;

MarmaladeSound*		AudioFindSoundMarmalade( const char* szSoundName )
{
MarmaladeSound*		pSounds = mpMarmaladeSoundList;

	while( pSounds )
	{
		// todo - to optimise, replace with hash
		if ( stricmp( pSounds->mszSoundName, szSoundName ) == 0 )
		{
			return( pSounds );
		}
		pSounds = pSounds->mpNext;
	}
	return( NULL );
}

// Here.. szFilename would refer to a text/list of sound files to be referenced by the game
void		AudioSoundLoad( const char* szFilename )
{
/*
*/
}

void		AudioSoundLoadSingle( const char* szSoundName, const char* pcSoundFilename )
{
MarmaladeSound*		pSound = new MarmaladeSound;
s3eFile*	pFileHandle = s3eFileOpen(pcSoundFilename, "rb");
int			nSoundDataMemSize; 
int			nLoop;
ushort*		puwOutRunner;
ushort*		puwTempBuffer;

	pSound->mnSoundFileSize = s3eFileGetSize(pFileHandle);
	// Assume file is mono
	nSoundDataMemSize = pSound->mnSoundFileSize * 2;
	pSound->mpuwSoundData = (ushort*)s3eMallocBase( nSoundDataMemSize );
	puwTempBuffer = (ushort*)s3eMallocBase( pSound->mnSoundFileSize );

	s3eFileRead(puwTempBuffer, pSound->mnSoundFileSize, 1, pFileHandle);
    s3eFileClose(pFileHandle);

	puwOutRunner = pSound->mpuwSoundData;

	for( nLoop = 0; nLoop < pSound->mnSoundFileSize / 2; nLoop++ )
	{
		*puwOutRunner = puwTempBuffer[nLoop];
		puwOutRunner++;
		*puwOutRunner = puwTempBuffer[nLoop];
		puwOutRunner++;
	}

	s3eFreeBase( puwTempBuffer );

	pSound->mszSoundName = (char*)( SystemMalloc( strlen( szSoundName ) + 1 ) );
	strcpy( pSound->mszSoundName, szSoundName );

	pSound->mpNext = mpMarmaladeSoundList;
	mpMarmaladeSoundList = pSound;
}


void		AudioSoundPlay( const char* szSoundName, float fVolume )
{
MarmaladeSound*		pSound = AudioFindSoundMarmalade( szSoundName );

	if ( pSound )
	{
	int		hOutputChannel;
		
		hOutputChannel = s3eSoundGetFreeChannel();
		s3eSoundChannelPlay( hOutputChannel, (int16*)pSound->mpuwSoundData, pSound->mnSoundFileSize / 4, 1, 0);		// ,1, 0 = play once
	}

//    g_Channel = s3eSoundGetFreeChannel();
}


void		AudioInit( void )
{
    s3eSoundSetInt(S3E_SOUND_DEFAULT_FREQ, 11000);
/*
    // Finds a free channel that we can use to play our raw file on.
    g_Channel = s3eSoundGetFreeChannel();
    // Setting default frequency at which all channels will play at, in Hz.
*/
}

void		AudioUpdate( float delta )
{


}

void		AudioShutdown( void )
{


}

