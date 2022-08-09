#ifndef GAMECOMMON_AUDIO_H
#define GAMECOMMON_AUDIO_H


extern void		AudioInit( void );

extern void		AudioUpdate( float delta );

extern void		AudioShutdown( void );


//----------------------------------------
// For music playback

extern void		AudioMusicPlay( const char* pcFileName, int nFullFileLength, int nArchiveHandle );
extern void		AudioMusicStop( void );

//-----------------------------------------
// For sound effects

extern void		AudioSoundLoadSoundset( const char* pcSoundsetFilename );

extern void		AudioSoundLoadSingle( const char* szSoundName, const char* pcSoundFilename );

extern void		AudioSoundPlay( const char* szSoundName, float fVolume );

//---------------------------------------


#endif