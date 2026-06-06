#include "StdWinInclude.h"
#include <mmsystem.h>
#include <vector>

#include "StandardDef.h"
#include "Engine.h"
#include "Interface.h"
//----------------------------------------
// Should be separated from the milk stuff and kept as separate audio utils
#include "../../Milkdrop/audio/common.h"
#include "../../Milkdrop/vis_milk2/utility.h"

//----------------------------------------
#include "AudioInModule.h"

#define SAMPLE_SIZE 576


static std::mutex pcmMutex;
static unsigned char pcmLeftIn[SAMPLE_SIZE];
static unsigned char pcmRightIn[SAMPLE_SIZE];
static unsigned char pcmLeftOut[SAMPLE_SIZE];
static unsigned char pcmRightOut[SAMPLE_SIZE];

LoopbackCaptureThreadFunctionArguments mLoopbackThreadArgs;

//#define	MIT_ANALYSIS_VERS

const float		fBaseSampleFreq = 48000.0f;
const float		fFFTSampleFreq = 12000.0f;


int		AudioInModule::FrequencyToSpectrumIndex( float freq )
{
int		slot = (int)((NUM_FREQUENCIES * (freq))/fFFTSampleFreq);

	slot = ClampInt(slot, 0, NUM_FREQUENCIES - 1);
	return(slot);

}
void		AudioInModule::GetFFTSpectrumRange( float fMinFreq, float fMaxFreq, int* pnSpectrumStartOut, int* pnSpectrumEndOut )
{
float min_freq = fMinFreq;
float max_freq = fMaxFreq;
float net_octaves = (logf(max_freq/min_freq) / logf(2.0f));     // 5.7846348455575205777914165223593
float octaves_per_band = net_octaves / 3.0f;                    // 1.9282116151858401925971388407864
float mult = powf(2.0f, octaves_per_band); // each band's highest freq. divided by its lowest freq.; 3.805831305510122517035102576162
int start, end;

	//   bass:  200-761
	//   mids:  761-2897
	//   treb:  2897-11025
	start = (int)((NUM_FREQUENCIES * min_freq)/fFFTSampleFreq);
	end   = (int)((NUM_FREQUENCIES * max_freq)/fFFTSampleFreq);
	if (start < 0) start = 0;
	if (end > NUM_FREQUENCIES) end = NUM_FREQUENCIES;

	*pnSpectrumStartOut = start;
	*pnSpectrumEndOut = end;
}



void AudioInModule::AnalyzeNewSound(unsigned char *pWaveL, unsigned char *pWaveR)
{
	// we get 576 samples in from winamp.
	// the output of the fft has 'num_frequencies' samples,
	//   and represents the frequency range 0 hz - 22,050 hz.
	// usually, plugins only use half of this output (the range 0 hz - 11,025 hz),
	//   since >10 khz doesn't usually contribute much.

	int i;

	float temp_wave[2][576];
	float	fFPS = InterfaceGetFPS();//	MilkGetFPS();
	float	fftGain = 1.0f;

	int old_i = 0;
	for (i=0; i<576; i++)
	{
		m_sound.fWaveform[0][i] = (float)((pWaveL[i] ^ 128) - 128);
		m_sound.fWaveform[1][i] = (float)((pWaveR[i] ^ 128) - 128);

		// simulating single frequencies from 200 to 11,025 Hz:
		//float freq = 1.0f + 11050*(GetFrame() % 100)*0.01f;
		//m_sound.fWaveform[0][i] = 10*sinf(i*freq*6.28f/44100.0f);

		// damp the input into the FFT a bit, to reduce high-frequency noise:
		temp_wave[0][i] = 0.5f*(m_sound.fWaveform[0][i] + m_sound.fWaveform[0][old_i]);
		temp_wave[1][i] = 0.5f*(m_sound.fWaveform[1][i] + m_sound.fWaveform[1][old_i]);

		temp_wave[0][i] *= fftGain;
		temp_wave[1][i] *= fftGain;
		
		old_i = i;

	}

	m_fftobj.time_to_frequency_domain(temp_wave[0], m_sound.fSpectrum[0]);
	m_fftobj.time_to_frequency_domain(temp_wave[1], m_sound.fSpectrum[1]);

	// sum (left channel) spectrum up into 3 bands
	// [note: the new ranges do it so that the 3 bands are equally spaced, pitch-wise]
	float min_freq = 20.0f;
	float max_freq = fFFTSampleFreq;
	float net_octaves = (logf(max_freq/min_freq) / logf(2.0f));     // 5.7846348455575205777914165223593
	float octaves_per_band = net_octaves / 3.0f;                    // 1.9282116151858401925971388407864
	float mult = powf(2.0f, octaves_per_band); // each band's highest freq. divided by its lowest freq.; 3.805831305510122517035102576162
	int ch;

	int start, end;

	// [to verify: min_freq * mult * mult * mult should equal max_freq.]
	for (ch=0; ch<2; ch++)
	{
		for (i=0; i<3; i++)
		{
			// old guesswork code for this:
			//   float exp = 2.1f;
			//   int start = (int)(NUM_FREQUENCIES*0.5f*powf(i/3.0f, exp));
			//   int end   = (int)(NUM_FREQUENCIES*0.5f*powf((i+1)/3.0f, exp));
			// results:
			//          old range:      new range (ideal):
			//   bass:  0-1097          200-761
			//   mids:  1097-4705       761-2897
			//   treb:  4705-11025      2897-11025
			start = (int)(NUM_FREQUENCIES * min_freq*powf(mult, (float)i)/fFFTSampleFreq);
			end   = (int)(NUM_FREQUENCIES * min_freq*powf(mult, (float)(i+1))/fFFTSampleFreq);
			if (start < 0) start = 0;
			if (end > NUM_FREQUENCIES) end = NUM_FREQUENCIES;

			m_sound.imm[ch][i] = 0;
			for (int j=start; j<end; j++)
				m_sound.imm[ch][i] += m_sound.fSpectrum[ch][j];
			m_sound.imm[ch][i] /= (float)(end-start);
		}
	}

	// multiply by long-term, empirically-determined inverse averages:
	// (for a trial of 244 songs, 10 seconds each, somewhere in the 2nd or 3rd minute,
	//  the average levels were: 0.326781557	0.38087377	0.199888934
	for (ch=0; ch<2; ch++)
	{
		m_sound.imm[ch][0] /= 0.326781557f;//0.270f;
		m_sound.imm[ch][1] /= 0.380873770f;//0.343f;
		m_sound.imm[ch][2] /= 0.199888934f;//0.295f;
	}

	// do temporal blending to create attenuated and super-attenuated versions
	for (ch=0; ch<2; ch++)
	{
		for (i=0; i<3; i++)
		{
			// m_sound.avg[i]
			{
				float avg_mix;
				if (m_sound.imm[ch][i] > m_sound.avg[ch][i])
					avg_mix = AdjustRateToFPS(0.2f, 14.0f, fFPS);
				else
					avg_mix = AdjustRateToFPS(0.5f, 14.0f, fFPS);
				m_sound.avg[ch][i] = m_sound.avg[ch][i]*avg_mix + m_sound.imm[ch][i]*(1-avg_mix);
			}

			// m_sound.med_avg[i]
			// m_sound.long_avg[i]
			{
				float med_mix  = 0.91f;//0.800f + 0.11f*powf(t, 0.4f);    // primarily used for velocity_damping
				float long_mix = 0.96f;//0.800f + 0.16f*powf(t, 0.2f);    // primarily used for smoke plumes
				med_mix  = AdjustRateToFPS(med_mix, 14.0f, fFPS);
				long_mix = AdjustRateToFPS(long_mix, 14.0f, fFPS);
				m_sound.med_avg[ch][i]  =  m_sound.med_avg[ch][i]*(med_mix) + m_sound.imm[ch][i]*(1-med_mix);
				m_sound.long_avg[ch][i] = m_sound.long_avg[ch][i]*(long_mix) + m_sound.imm[ch][i]*(1-long_mix);
			}
		}
	}
}

int		AudioInModule::CreateThreads()
{
	int argc = 1;
    LPCWSTR argv[4] = { L"", L"--file", L"loopback-capture.wav", L"--int-16" };
    HRESULT hr = S_OK;

    // parse command line
    CPrefs prefs(argc, argv, hr);


   // create a "loopback capture has started" event
    HANDLE hStartedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hStartedEvent) 
	{
//        ERR(L"CreateEvent failed: last error is %u", GetLastError());
        return -__LINE__;
    }
    static CloseHandleOnExit closeStartedEvent(hStartedEvent);

    // create a "stop capturing now" event
    HANDLE hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hStopEvent) {
  //      ERR(L"CreateEvent failed: last error is %u", GetLastError());
        return -__LINE__;
    }
    static CloseHandleOnExit closeStopEvent(hStopEvent);

    // create arguments for loopback capture thread
    mLoopbackThreadArgs.hr = E_UNEXPECTED; // thread will overwrite this
    mLoopbackThreadArgs.pMMDevice = prefs.m_pMMDevice;
    mLoopbackThreadArgs.bInt16 = prefs.m_bInt16;
    mLoopbackThreadArgs.hFile = prefs.m_hFile;
    mLoopbackThreadArgs.hStartedEvent = hStartedEvent;
    mLoopbackThreadArgs.hStopEvent = hStopEvent;
    mLoopbackThreadArgs.nFrames = 0;

    HANDLE hThread = CreateThread(
        NULL, 0,
        LoopbackCaptureThreadFunction, &mLoopbackThreadArgs,
        0, NULL
    );
    if (NULL == hThread) 
	{
		SysDebugPrint( "CreateThread failed: last error is %u", GetLastError());
        return -__LINE__;
    }
    static CloseHandleOnExit closeThread(hThread);

    // wait for either capture to start or the thread to end
    HANDLE waitArray[2] = { hStartedEvent, hThread };
    DWORD dwWaitResult;
    dwWaitResult = WaitForMultipleObjects(
        ARRAYSIZE(waitArray), waitArray,
        FALSE, INFINITE
    );

    if (WAIT_OBJECT_0 + 1 == dwWaitResult) 
	{
		SysDebugPrint( "Thread aborted before starting to loopback capture: hr = 0x%08x", mLoopbackThreadArgs.hr);
        return -__LINE__;
    }

    if (WAIT_OBJECT_0 != dwWaitResult) 
	{
        SysDebugPrint( "Unexpected WaitForMultipleObjects return value %u", dwWaitResult);
        return -__LINE__;
    }

    // at this point capture is running
	return( 0 );
}

void		AudioInModule::OnInitialise( void )
{
	int		i, ch;

	ZeroMemory(&m_sound, sizeof(td_soundinfo));

	for ( ch=0; ch<2; ch++)
	{
		for (i=0; i<3; i++)
		{
			m_sound.infinite_avg[ch][i] = m_sound.avg[ch][i] = m_sound.med_avg[ch][i] = m_sound.long_avg[ch][i] = 1.0f;
		}
	}
	
	m_fftobj.Init(576, NUM_FREQUENCIES);

	CreateThreads();

}


void		AudioInModule::HighFreqUpdate()
{
}


void		AudioInModule::OnUpdate( float Delta )
{
	// Copy the latest audio data buffer
//    memset(pcmLeftIn, 0, SAMPLE_SIZE);
//    memset(pcmRightIn, 0, SAMPLE_SIZE);
	if ( GetAudioBuf(pcmLeftIn, pcmRightIn, SAMPLE_SIZE) == true )
	{
		std::unique_lock<std::mutex> lock(pcmMutex);
	    memcpy(pcmLeftOut, pcmLeftIn, SAMPLE_SIZE);
		memcpy(pcmRightOut, pcmRightIn, SAMPLE_SIZE);
		AnalyzeNewSound( pcmLeftOut, pcmRightOut );
	}

}

void 	AudioInModule::OnShutdown()
{
	m_fftobj.CleanUp();

}

BYTE*		AudioInModule::GetAudioBufOutLeft()
{
	return( pcmLeftOut );
}

BYTE*		AudioInModule::GetAudioBufOutRight()
{
	return( pcmRightOut );
}

int			AudioInModule::GetAudioSampleSize()
{
	return SAMPLE_SIZE;
}

float		AudioInModule::GetAnalysisValue( int dampingLevel, int type )
{
	switch( dampingLevel )
	{
	case 0:
		return( (m_sound.imm[0][type] + m_sound.imm[1][type]) * 0.5f );
		break;
	case 1:
	default:
		return( (m_sound.avg[0][type] + m_sound.avg[1][type]) * 0.5f );
		break;
	case 2:
		return( (m_sound.med_avg[0][type] + m_sound.med_avg[1][type]) * 0.5f );
		break;
	case 3:
		return( (m_sound.long_avg[0][type] + m_sound.long_avg[1][type]) * 0.5f );
		break;
	}
	return( 0.0f );
}


float		AudioInModule::GetCustomRangeValue( float fMinFreq, float fMaxFreq, int dampingLevel )
{
int		start = FrequencyToSpectrumIndex(fMinFreq);
int		end = FrequencyToSpectrumIndex(fMaxFreq);
int		nNumSlots = end - start;
float		fTot = 0.0f;

	if ( nNumSlots == 0 )
	{
		end = start + 1;
		nNumSlots = 1;
	}
	
	for ( int i = start; i < end; i++ )
	{
		fTot += ( (m_sound.fSpectrum[0][i] + m_sound.fSpectrum[1][i]) * 0.5f );
	}
	fTot /= nNumSlots;
	return fTot;
}

float		AudioInModule::GetBassValue( int dampingLevel )
{
	return( GetAnalysisValue( dampingLevel, 0 ));
}

float		AudioInModule::GetMidValue( int dampingLevel )
{
	return( GetAnalysisValue( dampingLevel, 1 ));
}

float		AudioInModule::GetTrebleValue( int dampingLevel )
{
	return( GetAnalysisValue( dampingLevel, 2 ));

}

