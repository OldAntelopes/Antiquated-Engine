#ifndef DXV_READER_H
#define DXV_READER_H

#include <stdio.h>
#include <vector>

struct IMFSample;

// Minimal self-contained reader for Resolume DXV (DXV2/DXV3 normal quality) in QuickTime .mov files.
// Produces IMFSamples containing raw DXT1/DXT5 block data (row-major 4x4 blocks, texture size aligned to 16)
// suitable for uploading directly into a DXT texture.
class DXVReader
{
public:
	DXVReader() {}
	~DXVReader();

	bool		Open( const char* szFilename );
	void		Close();

	int			GetWidth() const { return mnWidth; }
	int			GetHeight() const { return mnHeight; }
	int			GetTextureWidth() const { return mnTexWidth; }
	int			GetTextureHeight() const { return mnTexHeight; }
	bool		IsDXT5() const { return mbDXT5; }
	// HQ (YCoCg) files are decoded on the CPU and returned as BGRA pixels (texture width * height * 4)
	bool		IsRGBA() const { return mbHQ; }
	bool		HasAlpha() const { return mbHQAlpha || mbDXT5; }
	float		GetDurationSec() const { return mfDurationSec; }

	// Mirrors IMFSourceReader::ReadSample - sets MF_SOURCE_READERF_ENDOFSTREAM in *pFlags at the end
	HRESULT		ReadSample( DWORD* pFlags, LONGLONG* pTimestamp, IMFSample** ppSample );
	// Positions the reader so the next ReadSample returns the frame at (or immediately before) the timestamp
	bool		Seek( u64 ullTimestamp );

private:
	struct Frame
	{
		u64		ullFileOffset;
		uint32	ulSize;
		u64		ullTimestamp;		// 100ns units
	};

	bool		DecodeFrame( const BYTE* pSrc, uint32 ulSrcLen, BYTE* pDest );
	void		ConvertHQToBGRA( BYTE* pDest );

	FILE*				mpFile = NULL;
	std::vector<Frame>	maFrames;
	std::vector<BYTE>	maReadBuffer;
	size_t				mnNextFrame = 0;
	int					mnWidth = 0;
	int					mnHeight = 0;
	int					mnTexWidth = 0;
	int					mnTexHeight = 0;
	uint32				mulTexSize = 0;
	bool				mbDXT5 = false;
	bool				mbHQ = false;
	bool				mbHQAlpha = false;
	uint32				mulYTexSize = 0;		// HQ: BC4 (Y) or Y+A texture
	uint32				mulCTexSize = 0;		// HQ: half-res CoCg texture
	std::vector<BYTE>	maYTex;
	std::vector<BYTE>	maCTex;
	std::vector<BYTE>	maCgPlane;
	std::vector<BYTE>	maCoPlane;
	float				mfDurationSec = 0.0f;
};

#endif
