// audiobuf.h

#include <mutex>
#include <windows.h>

// Reset audio buffer discarding stored audio data
void ResetAudioBuf();

// Return previously saved audio data for visualizer
bool GetAudioBuf(unsigned char *pWaveL, unsigned char *pWaveR, int SamplesCount);

// Pop exactly SamplesCount *new, contiguous* 48khz samples for analysis (e.g. BPM detection).
// Each captured sample is returned exactly once. Returns false if not enough new samples yet.
bool GetAudioBufContiguous(unsigned char *pWaveL, unsigned char *pWaveR, int SamplesCount);

// Save audio data for visualizer
void SetAudioBuf(const BYTE *pData, const UINT32 nNumFramesToRead, const WAVEFORMATEX *pwfx, const bool bInt16, int nBufferSize );

void	ShutdownAudioBuf();