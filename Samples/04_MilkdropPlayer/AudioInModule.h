
#ifndef AUDIO_IN_MODULE_H
#define AUDIO_IN_MODULE_H

#include "../../Milkdrop/vis_milk2/fft.h"
#include "../../Milkdrop/audio/soundinfo.h"


class AudioInModule
{
public:
	void		OnInitialise();
	void		OnUpdate( float Delta );
	void		OnShutdown( );
		
	BYTE*		GetAudioBufOutLeft();
	BYTE*		GetAudioBufOutRight();
	int			GetAudioSampleSize();

	float		GetBassValue( int dampingLevel );
	float		GetMidValue( int dampingLevel );
	float		GetTrebleValue( int dampingLevel );

	float		GetCustomRangeValue( float fMinFreq, float fMaxFreq, int dampingLevel );

	void		HighFreqUpdate();

	static void		GetFFTSpectrumRange( float fMinFreq, float fMaxFreq, int* pnSpectrumStartOut, int* pnSpectrumEndOut );

	td_soundinfo*		GetSoundInfo() { return( &m_sound ); }
private:
	void		AnalyzeNewSound(unsigned char *pWaveL, unsigned char *pWaveR);
	float		GetAnalysisValue( int dampingLevel, int type );
	int			FrequencyToSpectrumIndex( float freq );

	int		CreateThreads();
	td_soundinfo		m_sound;
	td_soundinfo		m_ElementalSound;
	FFT   m_fftobj;

};



#endif // !AUDIO_IN_MODULE_H
