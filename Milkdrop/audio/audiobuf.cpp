// audiobuf.cpp

#include "audiobuf.h"
#include <stdio.h>

#include "../../Pub/Include/StandardDef.h"

//#define		STORE_SAMPLES_FOR_TEST	

static FILE*	s_pRecordFile = NULL;


#define SAMPLE_SIZE_LPB 576 // Max number of audio samples stored in circular buffer. Should be no less than SAMPLE_SIZE. Expected sampling rate is 44100 Hz or 48000 Hz (samples per second).

std::mutex pcmLpbMutex;
unsigned char pcmLeftLpb[SAMPLE_SIZE_LPB]; // Circular buffer (left channel)
unsigned char pcmRightLpb[SAMPLE_SIZE_LPB]; // Circular buffer (right channel)
bool pcmBufDrained = false; // Buffer drained by visualization thread and holds no new samples
signed int pcmLen = 0; // Actual number of samples the buffer holds. Can be less than SAMPLE_SIZE_LPB
signed int pcmPos = 0; // Position to write new data

// Contiguous FIFO for analysis (e.g. BPM detection). Unlike the visualiser ring above - which
// always hands back the *latest* SAMPLE_SIZE_LPB samples, overlapping from one read to the next -
// every captured sample passes through this FIFO exactly once, so consumers can rely on each
// block they read being the next consecutive chunk of real time.
#define ANALYSIS_FIFO_SIZE	16384	// ~340ms @ 48khz - plenty of slack for a slow/stalled frame
static unsigned char	s_aucFifoL[ANALYSIS_FIFO_SIZE];
static unsigned char	s_aucFifoR[ANALYSIS_FIFO_SIZE];
static int				s_nFifoRead = 0;
static int				s_nFifoCount = 0;

// Caller must hold pcmLpbMutex
static void AnalysisFifoPush(unsigned char ucL, unsigned char ucR)
{
    if ( s_nFifoCount == ANALYSIS_FIFO_SIZE )
    {
        // Consumer has fallen too far behind - drop the oldest sample (stream is no longer contiguous)
        s_nFifoRead = (s_nFifoRead + 1) % ANALYSIS_FIFO_SIZE;
        s_nFifoCount--;
    }
    int	nWrite = (s_nFifoRead + s_nFifoCount) % ANALYSIS_FIFO_SIZE;
    s_aucFifoL[nWrite] = ucL;
    s_aucFifoR[nWrite] = ucR;
    s_nFifoCount++;
}

bool GetAudioBufContiguous(unsigned char *pWaveL, unsigned char *pWaveR, int SamplesCount)
{
    std::unique_lock<std::mutex> lock(pcmLpbMutex);
    if ( s_nFifoCount < SamplesCount )
    {
        return false;
    }
    for ( int i = 0; i < SamplesCount; i++ )
    {
        pWaveL[i] = s_aucFifoL[s_nFifoRead];
        pWaveR[i] = s_aucFifoR[s_nFifoRead];
        s_nFifoRead = (s_nFifoRead + 1) % ANALYSIS_FIFO_SIZE;
    }
    s_nFifoCount -= SamplesCount;
    return true;
}

void ResetAudioBuf() 
{
    std::unique_lock<std::mutex> lock(pcmLpbMutex);
    memset(pcmLeftLpb, 0, SAMPLE_SIZE_LPB);
    memset(pcmRightLpb, 0, SAMPLE_SIZE_LPB);
    pcmBufDrained = false;
    pcmLen = 0;
    pcmPos = 0;
    s_nFifoRead = 0;
    s_nFifoCount = 0;
}

bool GetAudioBuf(unsigned char *pWaveL, unsigned char *pWaveR, int SamplesCount) 
{
    std::unique_lock<std::mutex> lock(pcmLpbMutex);
    if ((pcmLen < SamplesCount) || (pcmBufDrained)) {
        // Buffer underrun. Insufficient new samples in circular buffer (pcmLeftLpb, pcmRightLpb)
//        memset(pWaveL, 0, SamplesCount);
 //       memset(pWaveR, 0, SamplesCount);
		return false;
    }
    else {
        // Circular buffer (pcmLeftLpb, pcmRightLpb) hold enough samples in it
        for (int i = 0; i < SAMPLE_SIZE_LPB; i++) {
            // int8_t [-128 .. +127] stored into uint8_t [0..255]
            pWaveL[i % SamplesCount] = pcmLeftLpb[(pcmPos + i) % SAMPLE_SIZE_LPB];
            pWaveR[i % SamplesCount] = pcmRightLpb[(pcmPos + i) % SAMPLE_SIZE_LPB];
        }
        pcmBufDrained = true;
		return true;
    }
}

int8_t FltToInt(float flt) 
{
    if (flt >= 1.0f) {
        return +127; // 0x7f
    }
    if (flt < -1.0f) {
        return -128; // 0x80
    }
    return (int8_t)(flt * 128);
}

// Union type for sample conversion
union u_type
{
    int32_t IntVar;
    float FltVar;
    uint8_t Bytes[4];
};

int8_t GetChannelSample(const BYTE *pData, int BlockOffset, int ChannelOffset, const bool bInt16) 
{
    u_type sample;

    sample.IntVar = 0;
    sample.Bytes[0] = pData[BlockOffset + ChannelOffset + 0];
    sample.Bytes[1] = pData[BlockOffset + ChannelOffset + 1];
    if (!bInt16) {
        sample.Bytes[2] = pData[BlockOffset + ChannelOffset + 2];
        sample.Bytes[3] = pData[BlockOffset + ChannelOffset + 3];
    }

    if (!bInt16) {
        return FltToInt(sample.FltVar); //float [-1.0f .. +1.0f] range converted to int8_t [-128 .. +127] and later stored into uint8_t [0 .. 255]
    }
    else {
        return (signed char)(sample.IntVar / 256); //int16_t [-32768 .. +32767] range converted to int8_t [-128 .. +127] and later stored into uint8_t [0..255]
    }
}

// Returns a normalized float sample in the range [-1.0f, +1.0f] for interpolation.
float GetChannelSampleFloat(const BYTE *pData, int BlockOffset, int ChannelOffset, const bool bInt16)
{
    u_type sample;

    sample.IntVar = 0;
    sample.Bytes[0] = pData[BlockOffset + ChannelOffset + 0];
    sample.Bytes[1] = pData[BlockOffset + ChannelOffset + 1];
    if (!bInt16) {
        sample.Bytes[2] = pData[BlockOffset + ChannelOffset + 2];
        sample.Bytes[3] = pData[BlockOffset + ChannelOffset + 3];
    }

    if (!bInt16) {
        return sample.FltVar; // Already normalized float [-1.0f .. +1.0f]
    }
    else {
        return (float)sample.IntVar / 32768.0f; // int16_t [-32768 .. +32767] normalized to float
    }
}

// Expecting pData holds:
//   signed 16-bit (2 bytes) PCM, Little Endian
//   or
//   32-bit float (4 bytes) PCM
// Supported audio formats:
//   pwfx->nChannels;          /* ANY number of channels (i.e. mono, stereo...) */
//   pwfx->nSamplesPerSec;     
//   pwfx->nBlockAlign;        /* ANY block size of data */
//   pwfx->wBitsPerSample;     /* 16 or 32 number of bits per sample of mono data */

void SetAudioBuf(const BYTE *pData, const UINT32 nNumFramesToRead, const WAVEFORMATEX *pwfx, const bool bInt16, int nBufferSize)
{
    int BlockOffset;

    int8_t LeftSample8;
    int8_t RightSample8;


    std::unique_lock<std::mutex> lock(pcmLpbMutex);
    //memset(pcmLeftLpb, 0, SAMPLE_SIZE_LPB);
    //memset(pcmRightLpb, 0, SAMPLE_SIZE_LPB);

	float	fInputSampleStride = 1.0f;
//	SysDebugPrint("SetAudioBuf: nSamplesPerSec = %u, nNumFramesToRead = %u, nBufferSize = %d", pwfx->nSamplesPerSec, nNumFramesToRead, nBufferSize); ")

	if ( pwfx->nSamplesPerSec > 48000 )
	{
		fInputSampleStride = ((float)pwfx->nSamplesPerSec) / 48000;
	}


	// Number of (48khz-resampled) output samples this packet yields. Using the stride here keeps
	// every read inside pData - previously small packets used len = nNumFramesToRead regardless of
	// the stride, which read past the end of the packet at >48khz device rates.
	int	nOutputSamples = (int)( (float)nNumFramesToRead / fInputSampleStride );

	// The visualiser ring only holds SAMPLE_SIZE_LPB samples - if this packet has more, keep the
	// *newest* ones (the old code kept the oldest, contrary to its own comment).
	int	nSkipForRing = ( nOutputSamples > SAMPLE_SIZE_LPB ) ? ( nOutputSamples - SAMPLE_SIZE_LPB ) : 0;
	int	len = nOutputSamples - nSkipForRing;

	// Read buffer
	for ( int k = 0; k < nOutputSamples; k++ ) 
	{
		LeftSample8 = 0; // Init with silence (pData == NULL means a silent packet)
		RightSample8 = 0;
		if ( pData != NULL )
		{
			// Box-filter (average) all input frames that map onto this output sample, rather than
			// point-sampling every Nth frame - acts as a basic anti-alias filter when decimating
			// (e.g. 192khz -> 48khz), which otherwise folds HF content into the analysed band
			int	nFirst = (int)( (float)k * fInputSampleStride );
			int	nLast = (int)( (float)(k + 1) * fInputSampleStride );
			if ( nLast > (int)nNumFramesToRead ) nLast = (int)nNumFramesToRead;
			if ( nLast <= nFirst ) nLast = nFirst + 1;
			int	nSumL = 0;
			int	nSumR = 0;
			for ( int f = nFirst; f < nLast; f++ )
			{
				BlockOffset = f * pwfx->nBlockAlign;
				int8_t	l = GetChannelSample(pData, BlockOffset, 0, bInt16);
				int8_t	r = ( pwfx->nChannels >= 2 ) ? GetChannelSample(pData, BlockOffset, 1 * (pwfx->wBitsPerSample / 8), bInt16) : l;
				nSumL += l;
				nSumR += r;
			}
			LeftSample8 = (int8_t)( nSumL / (nLast - nFirst) );
			RightSample8 = (int8_t)( nSumR / (nLast - nFirst) );
		}

        // Saving audio data for visualizer
        // 8-bit signed integer in Two's Complement Representation stored in unsigned char array
        // int8_t[-128 .. + 127] stored into uint8_t[0 .. 255]
		// Every sample goes into the contiguous analysis FIFO (consumed exactly once by GetAudioBufContiguous)
		AnalysisFifoPush( (unsigned char)LeftSample8, (unsigned char)RightSample8 );

#ifdef STORE_SAMPLES_FOR_TEST
		if ( s_pRecordFile == NULL )
		{
			fopen_s( &s_pRecordFile, "audiosamplerecord.dat", "wb" );
		}
		if ( s_pRecordFile != NULL )
		{
			unsigned char	ucSample = (unsigned char)LeftSample8;
			fwrite( &ucSample, 1, 1, s_pRecordFile );
		}
#endif

		// Visualiser ring: append *after* the newest data (previously small packets were written at
		// pcmPos + (SAMPLE_SIZE_LPB - len), i.e. on top of the most recent samples).
		if ( k >= nSkipForRing )
		{
			int	nRingIndex = (pcmPos + (k - nSkipForRing)) % SAMPLE_SIZE_LPB;
			pcmLeftLpb[nRingIndex] = LeftSample8;
			pcmRightLpb[nRingIndex] = RightSample8;
		}
	}

    pcmBufDrained = false;
    pcmLen = (pcmLen + len <= SAMPLE_SIZE_LPB) ? (pcmLen + len) : (SAMPLE_SIZE_LPB);
    pcmPos = (pcmPos + len) % SAMPLE_SIZE_LPB;

}


void	ShutdownAudioBuf()
{
	if ( s_pRecordFile != NULL )
	{
		fclose( s_pRecordFile );
		s_pRecordFile = NULL;
	}
}