// audiobuf.h

#include <mutex>
#include <windows.h>

// Reset audio buffer discarding stored audio data
void ResetAudioBuf();

// Return previously saved audio data for visualizer
bool GetAudioBuf(unsigned char *pWaveL, unsigned char *pWaveR, int SamplesCount);

// Save audio data for visualizer
void SetAudioBuf(const BYTE *pData, const UINT32 nNumFramesToRead, const WAVEFORMATEX *pwfx, const bool bInt16, int nBufferSize );

// Get the actual sample rate from the last audio buffer that was set
int GetAudioBufActualSampleRate();
