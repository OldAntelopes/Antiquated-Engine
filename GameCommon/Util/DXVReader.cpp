#include "mfapi.h"
#include "mfidl.h"
#include "mfreadwrite.h"

#include "StandardDef.h"
#include "Interface.h"
#include "Engine.h"


#include "DXVReader.h"

//-------------------------------------------------------------------------------------------------
// Byte helpers
//-------------------------------------------------------------------------------------------------
static inline uint32 ReadBE16( const BYTE* p ) { return ( (uint32)p[0] << 8 ) | p[1]; }
static inline uint32 ReadBE32( const BYTE* p ) { return ( (uint32)p[0] << 24 ) | ( (uint32)p[1] << 16 ) | ( (uint32)p[2] << 8 ) | p[3]; }
static inline u64 ReadBE64( const BYTE* p ) { return ( (u64)ReadBE32( p ) << 32 ) | ReadBE32( p + 4 ); }
static inline uint32 ReadLE32( const BYTE* p ) { return (uint32)p[0] | ( (uint32)p[1] << 8 ) | ( (uint32)p[2] << 16 ) | ( (uint32)p[3] << 24 ); }

#define MOV_ATOM( a, b, c, d )	( ( (uint32)(a) << 24 ) | ( (uint32)(b) << 16 ) | ( (uint32)(c) << 8 ) | (uint32)(d) )
// DXV frame tags are stored so that a little-endian read gives this value
#define DXV_TAG( a, b, c, d )	( (uint32)(d) | ( (uint32)(c) << 8 ) | ( (uint32)(b) << 16 ) | ( (uint32)(a) << 24 ) )

class DXVByteReader
{
public:
	DXVByteReader( const BYTE* p, uint32 len ) : mp( p ), mpEnd( p + len ) {}

	uint32	Remaining() const { return (uint32)( mpEnd - mp ); }
	BYTE	Byte() { if ( mp + 1 > mpEnd ) { mbOverrun = true; return 0; } return *mp++; }
	uint32	LE16() { if ( mp + 2 > mpEnd ) { mbOverrun = true; mp = mpEnd; return 0; } uint32 v = mp[0] | ( mp[1] << 8 ); mp += 2; return v; }
	uint32	LE32() { if ( mp + 4 > mpEnd ) { mbOverrun = true; mp = mpEnd; return 0; } uint32 v = ReadLE32( mp ); mp += 4; return v; }

	bool	mbOverrun = false;
private:
	const BYTE*	mp;
	const BYTE*	mpEnd;
};

//-------------------------------------------------------------------------------------------------
// QuickTime / MP4 container parsing
//-------------------------------------------------------------------------------------------------
struct MovTrackInfo
{
	struct StscEntry { uint32 firstChunk; uint32 samplesPerChunk; };
	struct SttsEntry { uint32 count; uint32 delta; };

	uint32	handlerType = 0;
	uint32	codec = 0;
	uint32	timescale = 0;
	u64		duration = 0;
	int		width = 0;
	int		height = 0;
	uint32	fixedSampleSize = 0;
	uint32	sampleCount = 0;
	std::vector<uint32>		sampleSizes;
	std::vector<u64>		chunkOffsets;
	std::vector<StscEntry>	stsc;
	std::vector<SttsEntry>	stts;
};

static void MovParseAtoms( const BYTE* p, u64 len, MovTrackInfo* pTrack, std::vector<MovTrackInfo>& tracks )
{
	while ( len >= 8 )
	{
		u64		size = ReadBE32( p );
		uint32	type = ReadBE32( p + 4 );
		uint32	hdr = 8;

		if ( size == 1 )
		{
			if ( len < 16 ) return;
			size = ReadBE64( p + 8 );
			hdr = 16;
		}
		else if ( size == 0 )
		{
			size = len;
		}
		if ( ( size < hdr ) || ( size > len ) ) return;

		const BYTE*	body = p + hdr;
		u64			bodyLen = size - hdr;

		switch ( type )
		{
		case MOV_ATOM( 't','r','a','k' ):
			{
				MovTrackInfo	track;
				MovParseAtoms( body, bodyLen, &track, tracks );
				tracks.push_back( track );
			}
			break;
		case MOV_ATOM( 'm','d','i','a' ):
		case MOV_ATOM( 'm','i','n','f' ):
		case MOV_ATOM( 's','t','b','l' ):
			MovParseAtoms( body, bodyLen, pTrack, tracks );
			break;
		case MOV_ATOM( 'h','d','l','r' ):
			// QuickTime has a media handler (mhlr) in 'mdia' and a data handler (dhlr) in 'minf' - only the former gives the track type
			if ( pTrack && ( bodyLen >= 12 ) && ( ReadBE32( body + 4 ) != MOV_ATOM( 'd','h','l','r' ) ) )
			{
				pTrack->handlerType = ReadBE32( body + 8 );
			}
			break;
		case MOV_ATOM( 'm','d','h','d' ):
			if ( pTrack )
			{
				if ( ( body[0] == 1 ) && ( bodyLen >= 32 ) )
				{
					pTrack->timescale = ReadBE32( body + 20 );
					pTrack->duration = ReadBE64( body + 24 );
				}
				else if ( bodyLen >= 20 )
				{
					pTrack->timescale = ReadBE32( body + 12 );
					pTrack->duration = ReadBE32( body + 16 );
				}
			}
			break;
		case MOV_ATOM( 's','t','s','d' ):
			if ( pTrack && ( bodyLen >= 8 + 36 ) )
			{
				const BYTE*	entry = body + 8;
				pTrack->codec = ReadBE32( entry + 4 );
				pTrack->width = (int)ReadBE16( entry + 32 );
				pTrack->height = (int)ReadBE16( entry + 34 );
			}
			break;
		case MOV_ATOM( 's','t','t','s' ):
			if ( pTrack && ( bodyLen >= 8 ) )
			{
				uint32	count = ReadBE32( body + 4 );
				if ( 8 + (u64)count * 8 <= bodyLen )
				{
					for ( uint32 i = 0; i < count; i++ )
					{
						MovTrackInfo::SttsEntry	e = { ReadBE32( body + 8 + i * 8 ), ReadBE32( body + 12 + i * 8 ) };
						pTrack->stts.push_back( e );
					}
				}
			}
			break;
		case MOV_ATOM( 's','t','s','c' ):
			if ( pTrack && ( bodyLen >= 8 ) )
			{
				uint32	count = ReadBE32( body + 4 );
				if ( 8 + (u64)count * 12 <= bodyLen )
				{
					for ( uint32 i = 0; i < count; i++ )
					{
						MovTrackInfo::StscEntry	e = { ReadBE32( body + 8 + i * 12 ), ReadBE32( body + 12 + i * 12 ) };
						pTrack->stsc.push_back( e );
					}
				}
			}
			break;
		case MOV_ATOM( 's','t','s','z' ):
			if ( pTrack && ( bodyLen >= 12 ) )
			{
				pTrack->fixedSampleSize = ReadBE32( body + 4 );
				pTrack->sampleCount = ReadBE32( body + 8 );
				if ( ( pTrack->fixedSampleSize == 0 ) &&
					 ( 12 + (u64)pTrack->sampleCount * 4 <= bodyLen ) )
				{
					pTrack->sampleSizes.resize( pTrack->sampleCount );
					for ( uint32 i = 0; i < pTrack->sampleCount; i++ )
					{
						pTrack->sampleSizes[i] = ReadBE32( body + 12 + i * 4 );
					}
				}
			}
			break;
		case MOV_ATOM( 's','t','c','o' ):
		case MOV_ATOM( 'c','o','6','4' ):
			if ( pTrack && ( bodyLen >= 8 ) )
			{
				bool	b64 = ( type == MOV_ATOM( 'c','o','6','4' ) );
				uint32	count = ReadBE32( body + 4 );
				uint32	entrySize = b64 ? 8 : 4;
				if ( 8 + (u64)count * entrySize <= bodyLen )
				{
					pTrack->chunkOffsets.resize( count );
					for ( uint32 i = 0; i < count; i++ )
					{
						const BYTE*	e = body + 8 + i * entrySize;
						pTrack->chunkOffsets[i] = b64 ? ReadBE64( e ) : ReadBE32( e );
					}
				}
			}
			break;
		default:
			break;
		}

		p += size;
		len -= size;
	}
}

//-------------------------------------------------------------------------------------------------
// DXV texture decompression
//-------------------------------------------------------------------------------------------------

// Reads the next 2-bit op code (refilling from a 32-bit control word) and resolves the back-reference distance
static bool DXVCheckpoint( DXVByteReader& reader, uint32& value, int& state, uint32& op, uint32& idx, uint32 x, uint32 pos )
{
	if ( state == 0 )
	{
		if ( reader.Remaining() < 4 ) return false;
		value = reader.LE32();
		state = 16;
	}
	op = value & 0x3;
	value >>= 2;
	state--;

	switch ( op )
	{
	case 1:
		idx = x;
		break;
	case 2:
		idx = ( reader.Byte() + 2 ) * x;
		break;
	case 3:
		idx = ( reader.LE16() + 0x102 ) * x;
		break;
	default:
		break;
	}
	if ( ( op != 0 ) && ( idx > pos ) ) return false;
	return !reader.mbOverrun;
}

static bool DXVDecompressDXT1( const BYTE* pSrc, uint32 ulSrcLen, BYTE* pDest, uint32 ulTexSize )
{
	DXVByteReader	reader( pSrc, ulSrcLen );
	uint32*			out = (uint32*)pDest;
	uint32			numElements = ulTexSize / 4;
	uint32			value = 0, op = 0, idx = 0;
	int				state = 0;
	uint32			pos = 2;

	if ( numElements < 2 ) return false;
	out[0] = reader.LE32();
	out[1] = reader.LE32();

	while ( pos + 2 <= numElements )
	{
		if ( !DXVCheckpoint( reader, value, state, op, idx, 2, pos ) ) return false;

		if ( op )
		{
			out[pos] = out[pos - idx]; pos++;
			out[pos] = out[pos - idx]; pos++;
		}
		else
		{
			if ( !DXVCheckpoint( reader, value, state, op, idx, 2, pos ) ) return false;
			out[pos] = op ? out[pos - idx] : reader.LE32(); pos++;

			if ( !DXVCheckpoint( reader, value, state, op, idx, 2, pos ) ) return false;
			out[pos] = op ? out[pos - idx] : reader.LE32(); pos++;
		}
		if ( reader.mbOverrun ) return false;
	}
	return true;
}

static bool DXVDecompressDXT5( const BYTE* pSrc, uint32 ulSrcLen, BYTE* pDest, uint32 ulTexSize )
{
	DXVByteReader	reader( pSrc, ulSrcLen );
	uint32*			out = (uint32*)pDest;
	uint32			numElements = ulTexSize / 4;
	uint32			value = 0, op = 0, idx = 0;
	uint32			run = 0;
	int				state = 0;
	uint32			pos = 4;

	if ( numElements < 4 ) return false;
	out[0] = reader.LE32();
	out[1] = reader.LE32();
	out[2] = reader.LE32();
	out[3] = reader.LE32();

	while ( pos + 2 <= numElements )
	{
		if ( run )
		{
			run--;
			out[pos] = out[pos - 4]; pos++;
			out[pos] = out[pos - 4]; pos++;
		}
		else
		{
			if ( reader.Remaining() < 1 ) return false;
			if ( state == 0 )
			{
				value = reader.LE32();
				state = 16;
			}
			op = value & 0x3;
			value >>= 2;
			state--;

			switch ( op )
			{
			case 0:
				{
					// Long copy of 4-element runs from the previous block
					uint32	check = reader.Byte() + 1;
					if ( check == 256 )
					{
						uint32	probe;
						do
						{
							probe = reader.LE16();
							check += probe;
						} while ( ( probe == 0xFFFF ) && ( !reader.mbOverrun ) );
					}
					while ( ( check ) && ( pos + 4 <= numElements ) )
					{
						out[pos] = out[pos - 4]; pos++;
						out[pos] = out[pos - 4]; pos++;
						out[pos] = out[pos - 4]; pos++;
						out[pos] = out[pos - 4]; pos++;
						check--;
					}
					if ( reader.mbOverrun ) return false;
				}
				continue;
			case 1:
				{
					run = reader.Byte();
					if ( run == 255 )
					{
						uint32	probe;
						do
						{
							probe = reader.LE16();
							run += probe;
						} while ( ( probe == 0xFFFF ) && ( !reader.mbOverrun ) );
					}
					out[pos] = out[pos - 4]; pos++;
					out[pos] = out[pos - 4]; pos++;
				}
				break;
			case 2:
				idx = 8 + reader.LE16();
				if ( ( idx > pos ) || ( ( pos - idx ) + 2 > numElements ) ) return false;
				out[pos] = out[pos - idx]; pos++;
				out[pos] = out[pos - idx]; pos++;
				break;
			case 3:
				out[pos] = reader.LE32(); pos++;
				out[pos] = reader.LE32(); pos++;
				break;
			}
			if ( reader.mbOverrun ) return false;
		}

		if ( !DXVCheckpoint( reader, value, state, op, idx, 4, pos ) ) return false;
		if ( pos + 2 > numElements ) return false;

		if ( op )
		{
			out[pos] = out[pos - idx]; pos++;
			out[pos] = out[pos - idx]; pos++;
		}
		else
		{
			if ( !DXVCheckpoint( reader, value, state, op, idx, 4, pos ) ) return false;
			out[pos] = op ? out[pos - idx] : reader.LE32(); pos++;

			if ( !DXVCheckpoint( reader, value, state, op, idx, 4, pos ) ) return false;
			out[pos] = op ? out[pos - idx] : reader.LE32(); pos++;
		}
		if ( reader.mbOverrun ) return false;
	}
	return true;
}

static bool LZFDecompress( const BYTE* pSrc, uint32 ulSrcLen, BYTE* pDest, uint32 ulDestLen )
{
	const BYTE*	ip = pSrc;
	const BYTE*	ipEnd = pSrc + ulSrcLen;
	BYTE*		op = pDest;
	BYTE*		opEnd = pDest + ulDestLen;

	while ( ip < ipEnd )
	{
		uint32	ctrl = *ip++;

		if ( ctrl < 32 )
		{
			uint32	len = ctrl + 1;
			if ( ( ip + len > ipEnd ) || ( op + len > opEnd ) ) return false;
			memcpy( op, ip, len );
			op += len;
			ip += len;
		}
		else
		{
			uint32	len = ctrl >> 5;
			if ( len == 7 )
			{
				if ( ip >= ipEnd ) return false;
				len += *ip++;
			}
			if ( ip >= ipEnd ) return false;
			uint32	dist = ( ( ctrl & 0x1F ) << 8 ) + *ip++ + 1;
			len += 2;
			if ( ( dist > (uint32)( op - pDest ) ) || ( op + len > opEnd ) ) return false;
			const BYTE*	ref = op - dist;
			while ( len-- ) *op++ = *ref++;
		}
	}
	return ( op == opEnd );
}

//-------------------------------------------------------------------------------------------------
// DXV HQ (YCG6 / YG10) decompression - ported from FFmpeg libavcodec/dxv.c
//-------------------------------------------------------------------------------------------------
static inline uint32 DXV_RL16( const BYTE* p ) { return (uint32)p[0] | ( (uint32)p[1] << 8 ); }
static inline void DXV_WL16( BYTE* p, uint32 v ) { p[0] = (BYTE)v; p[1] = (BYTE)( v >> 8 ); }
static inline void DXV_WL32( BYTE* p, uint32 v ) { p[0] = (BYTE)v; p[1] = (BYTE)( v >> 8 ); p[2] = (BYTE)( v >> 16 ); p[3] = (BYTE)( v >> 24 ); }
static inline uint32 DXV_Hash16( uint32 v ) { return ( 0x9E3779B1u * ( v & 0xFFFF ) ) >> 24; }
static inline uint32 DXV_Hash24( const BYTE* p ) { return ( 0x9E3779B1u * ( ReadLE32( p ) & 0xFFFFFF ) ) >> 24; }

class DXVStream
{
public:
	DXVStream( const BYTE* p, uint32 len ) : mpStart( p ), mp( p ), mpEnd( p + len ) {}

	uint32		Left() const { return (uint32)( mpEnd - mp ); }
	uint32		Tell() const { return (uint32)( mp - mpStart ); }
	const BYTE*	Ptr() const { return mp; }
	void		Seek( uint32 pos ) { if ( pos > (uint32)( mpEnd - mpStart ) ) { mbOverrun = true; mp = mpEnd; } else { mp = mpStart + pos; } }
	void		Skip( uint32 n ) { if ( n > Left() ) { mbOverrun = true; mp = mpEnd; } else { mp += n; } }
	uint32		Peek8() const { return ( mp < mpEnd ) ? *mp : 0; }
	uint32		U8() { if ( Left() < 1 ) { mbOverrun = true; return 0; } return *mp++; }
	uint32		LE16() { if ( Left() < 2 ) { mbOverrun = true; mp = mpEnd; return 0; } uint32 v = DXV_RL16( mp ); mp += 2; return v; }
	uint32		LE32() { if ( Left() < 4 ) { mbOverrun = true; mp = mpEnd; return 0; } uint32 v = ReadLE32( mp ); mp += 4; return v; }

	bool	mbOverrun = false;
private:
	const BYTE*	mpStart;
	const BYTE*	mp;
	const BYTE*	mpEnd;
};

struct DXVOpcodeEntry
{
	int		next;
	BYTE	val1;
	BYTE	val2;
};

static bool DXVHQFillLTable( DXVStream& gb, uint32* table, int* pnElements )
{
	uint32	half = 512, bits = 1023, left = 1024;
	int		counter = 0, rshift = 10, lshift = 30;
	uint32	mask = gb.LE32() >> 2;

	while ( left )
	{
		if ( counter >= 256 ) return false;
		uint32	value = bits & mask;
		left -= value;
		mask >>= rshift;
		lshift -= rshift;
		table[counter++] = value;
		if ( lshift < 16 )
		{
			if ( gb.Left() == 0 ) return false;
			uint32	input = gb.LE16();
			mask += input << lshift;
			lshift += 16;
		}
		if ( left < half )
		{
			half >>= 1;
			bits >>= 1;
			rshift--;
		}
	}

	while ( ( counter > 0 ) && ( table[counter - 1] == 0 ) ) counter--;
	if ( counter <= 0 ) return false;

	*pnElements = counter;
	if ( counter < 256 ) memset( &table[counter], 0, sizeof( uint32 ) * ( 256 - counter ) );
	if ( lshift >= 16 ) gb.Seek( gb.Tell() - 2 );
	return !gb.mbOverrun;
}

static bool DXVHQFillOpTable( const uint32* table0, DXVOpcodeEntry* table1, int nElements )
{
	uint32	table2[256] = { 0 };

	table2[0] = table0[0];
	for ( int i = 0; i < nElements - 1; i++ )
	{
		table2[i + 1] = table0[i + 1] + table2[i];
	}

	int		k = 0;
	if ( !table2[0] )
	{
		do { k++; } while ( ( k < 256 ) && ( !table2[k] ) );
		if ( k >= 256 ) return false;
	}

	uint32	x = 0;
	uint32	j = 2;
	for ( int i = 1024; i > 0; i-- )
	{
		table1[x].val1 = (BYTE)k;
		while ( ( k < 256 ) && ( j > table2[k] ) ) k++;
		x = ( x - 383 ) & 0x3FF;
		j++;
	}

	memcpy( table2, table0, sizeof( uint32 ) * nElements );

	for ( int i = 0; i < 1024; i++ )
	{
		uint32	val0 = table1[i].val1;
		uint32	val1 = table2[val0];
		table2[val0]++;
		if ( val1 == 0 ) return false;
		int		log2 = 0;
		while ( ( val1 >> ( log2 + 1 ) ) != 0 ) log2++;
		if ( log2 > 10 ) return false;
		table1[i].val2 = (BYTE)( 10 - log2 );
		table1[i].next = (int)( val1 << table1[i].val2 ) - 1024;
	}
	return true;
}

static bool DXVHQGetOpcodes( DXVStream& gb, const uint32* table, BYTE* dst, uint32 opSize, int nElements )
{
	DXVOpcodeEntry	optable[1024];
	if ( !DXVHQFillOpTable( table, optable, nElements ) ) return false;

	const BYTE*	src = gb.Ptr();
	uint32		sizeInBits = gb.LE32();
	u64			bytes = ( (u64)sizeInBits + 7 ) >> 3;
	if ( ( gb.mbOverrun ) || ( bytes <= 4 ) ) return false;
	uint32		endoffset = (uint32)( bytes - 4 );
	if ( gb.Left() < endoffset ) return false;

	uint32	offset = endoffset;
	uint32	next = ReadLE32( src + endoffset );
	int		rshift = ( ( ( sizeInBits & 0xFF ) - 1 ) & 7 ) + 15;
	int		lshift = 32 - rshift;
	uint32	idx = ( next >> rshift ) & 0x3FF;

	for ( uint32 i = 0; i < opSize; i++ )
	{
		dst[i] = optable[idx].val1;
		uint32	val = optable[idx].val2;
		uint32	sum = val + (uint32)lshift;
		uint32	x = ( ( next << lshift ) >> 1 ) >> ( 31 - val );
		if ( ( sum >> 3 ) > offset ) return false;
		offset -= ( sum >> 3 );
		lshift = (int)( sum & 7 );
		int		newIdx = (int)x + optable[idx].next;
		if ( ( newIdx < 0 ) || ( newIdx >= 1024 ) ) return false;
		idx = (uint32)newIdx;
		next = ReadLE32( src + offset );
	}

	gb.Skip( endoffset );
	return !gb.mbOverrun;
}

// Returns bytes consumed, or -1 on error
static int DXVHQDecompressOpcodes( DXVStream& gb, BYTE* dst, uint32 opSize )
{
	uint32	pos = gb.Tell();
	uint32	flag = gb.Peek8();

	if ( ( flag & 3 ) == 0 )
	{
		gb.Skip( 1 );
		if ( gb.Left() < opSize ) return -1;
		memcpy( dst, gb.Ptr(), opSize );
		gb.Skip( opSize );
	}
	else if ( ( flag & 3 ) == 1 )
	{
		gb.Skip( 1 );
		memset( dst, (int)gb.U8(), opSize );
	}
	else
	{
		uint32	table[256];
		int		nElements = 0;
		if ( !DXVHQFillLTable( gb, table, &nElements ) ) return -1;
		if ( !DXVHQGetOpcodes( gb, table, dst, opSize, nElements ) ) return -1;
	}
	if ( gb.mbOverrun ) return -1;
	return (int)( gb.Tell() - pos );
}

// Decodes one 8-byte BC4 half-block. 'offset' is 0 for a plain 8-byte stream, 8 when two streams are interleaved (16-byte blocks)
static bool DXVHQDecompressCGO( DXVStream& gb, BYTE* tex, uint32 texSize, const BYTE* op, uint32& oi, uint32 opSize,
								BYTE*& dst, int& state, BYTE** tab0, BYTE** tab1, int offset )
{
	const uint32	step = 8 + offset;
	uint32			done = (uint32)( dst - tex );
	if ( ( done < step ) || ( done + 8 > texSize ) ) return false;

#define DXV_TAB( t, out )	{ out = t[gb.U8()]; if ( !out ) return false; }
#define DXV_BACKREF( out )	{ uint32 vv_ = step * ( gb.LE16() + 1 ); if ( vv_ > done ) return false; out = dst - vv_; }
#define DXV_SET_T0()		tab0[DXV_Hash16( DXV_RL16( dst ) )] = dst
#define DXV_SET_T1()		tab1[DXV_Hash24( dst + 2 )] = dst + 2

	BYTE	*tptr0, *tptr1, *tptr3;
	bool	bCopyPrev = false;

	if ( state <= 0 )
	{
		if ( oi >= opSize ) return false;
		uint32	opcode = op[oi++];

		if ( opcode == 0 )
		{
			uint32	v = gb.U8();
			if ( v == 255 )
			{
				uint32	probe;
				do
				{
					if ( gb.Left() == 0 ) return false;
					probe = gb.LE16();
					v += probe;
				} while ( probe == 0xFFFF );
			}
			state = (int)( v + 4 );
			bCopyPrev = true;
		}
		else
		{
			switch ( opcode )
			{
			case 1:
				DXV_WL32( dst, ReadLE32( dst - step ) );
				DXV_WL32( dst + 4, ReadLE32( dst - step + 4 ) );
				break;
			case 2:
				DXV_BACKREF( tptr0 );
				DXV_WL32( dst, ReadLE32( tptr0 ) );
				DXV_WL32( dst + 4, ReadLE32( tptr0 + 4 ) );
				DXV_SET_T0(); DXV_SET_T1();
				break;
			case 3:
				DXV_WL32( dst, gb.LE32() );
				DXV_WL32( dst + 4, gb.LE32() );
				DXV_SET_T0(); DXV_SET_T1();
				break;
			case 4:
				DXV_TAB( tab1, tptr3 );
				DXV_WL16( dst, gb.LE16() );
				DXV_WL16( dst + 2, DXV_RL16( tptr3 ) );
				dst[4] = tptr3[2];
				DXV_WL16( dst + 5, gb.LE16() );
				dst[7] = (BYTE)gb.U8();
				DXV_SET_T0();
				break;
			case 5:
				DXV_TAB( tab1, tptr3 );
				DXV_WL16( dst, gb.LE16() );
				DXV_WL16( dst + 2, gb.LE16() );
				dst[4] = (BYTE)gb.U8();
				DXV_WL16( dst + 5, DXV_RL16( tptr3 ) );
				dst[7] = tptr3[2];
				DXV_SET_T0(); DXV_SET_T1();
				break;
			case 6:
				DXV_TAB( tab1, tptr0 );
				DXV_TAB( tab1, tptr1 );
				DXV_WL16( dst, gb.LE16() );
				DXV_WL16( dst + 2, DXV_RL16( tptr0 ) );
				dst[4] = tptr0[2];
				DXV_WL16( dst + 5, DXV_RL16( tptr1 ) );
				dst[7] = tptr1[2];
				DXV_SET_T0();
				break;
			case 7:
				DXV_BACKREF( tptr0 );
				DXV_WL16( dst, gb.LE16() );
				DXV_WL16( dst + 2, DXV_RL16( tptr0 + 2 ) );
				DXV_WL32( dst + 4, ReadLE32( tptr0 + 4 ) );
				DXV_SET_T0(); DXV_SET_T1();
				break;
			case 8:
				DXV_TAB( tab0, tptr1 );
				DXV_WL16( dst, DXV_RL16( tptr1 ) );
				DXV_WL16( dst + 2, gb.LE16() );
				DXV_WL32( dst + 4, gb.LE32() );
				DXV_SET_T1();
				break;
			case 9:
				DXV_TAB( tab0, tptr1 );
				DXV_TAB( tab1, tptr3 );
				DXV_WL16( dst, DXV_RL16( tptr1 ) );
				DXV_WL16( dst + 2, DXV_RL16( tptr3 ) );
				dst[4] = tptr3[2];
				DXV_WL16( dst + 5, gb.LE16() );
				dst[7] = (BYTE)gb.U8();
				DXV_SET_T1();
				break;
			case 10:
				DXV_TAB( tab0, tptr1 );
				DXV_TAB( tab1, tptr3 );
				DXV_WL16( dst, DXV_RL16( tptr1 ) );
				DXV_WL16( dst + 2, gb.LE16() );
				dst[4] = (BYTE)gb.U8();
				DXV_WL16( dst + 5, DXV_RL16( tptr3 ) );
				dst[7] = tptr3[2];
				DXV_SET_T1();
				break;
			case 11:
				DXV_TAB( tab0, tptr0 );
				DXV_TAB( tab1, tptr3 );
				DXV_TAB( tab1, tptr1 );
				DXV_WL16( dst, DXV_RL16( tptr0 ) );
				DXV_WL16( dst + 2, DXV_RL16( tptr3 ) );
				dst[4] = tptr3[2];
				DXV_WL16( dst + 5, DXV_RL16( tptr1 ) );
				dst[7] = tptr1[2];
				break;
			case 12:
				DXV_TAB( tab0, tptr1 );
				DXV_BACKREF( tptr0 );
				DXV_WL16( dst, DXV_RL16( tptr1 ) );
				DXV_WL16( dst + 2, DXV_RL16( tptr0 + 2 ) );
				DXV_WL32( dst + 4, ReadLE32( tptr0 + 4 ) );
				DXV_SET_T1();
				break;
			case 13:
				DXV_WL16( dst, DXV_RL16( dst - step ) );
				DXV_WL16( dst + 2, gb.LE16() );
				DXV_WL32( dst + 4, gb.LE32() );
				DXV_SET_T1();
				break;
			case 14:
				DXV_TAB( tab1, tptr3 );
				DXV_WL16( dst, DXV_RL16( dst - step ) );
				DXV_WL16( dst + 2, DXV_RL16( tptr3 ) );
				dst[4] = tptr3[2];
				DXV_WL16( dst + 5, gb.LE16() );
				dst[7] = (BYTE)gb.U8();
				DXV_SET_T1();
				break;
			case 15:
				DXV_TAB( tab1, tptr3 );
				DXV_WL16( dst, DXV_RL16( dst - step ) );
				DXV_WL16( dst + 2, gb.LE16() );
				dst[4] = (BYTE)gb.U8();
				DXV_WL16( dst + 5, DXV_RL16( tptr3 ) );
				dst[7] = tptr3[2];
				DXV_SET_T1();
				break;
			case 16:
				DXV_TAB( tab1, tptr3 );
				DXV_TAB( tab1, tptr1 );
				DXV_WL16( dst, DXV_RL16( dst - step ) );
				DXV_WL16( dst + 2, DXV_RL16( tptr3 ) );
				dst[4] = tptr3[2];
				DXV_WL16( dst + 5, DXV_RL16( tptr1 ) );
				dst[7] = tptr1[2];
				break;
			case 17:
				DXV_BACKREF( tptr0 );
				DXV_WL16( dst, DXV_RL16( dst - step ) );
				DXV_WL16( dst + 2, DXV_RL16( tptr0 + 2 ) );
				DXV_WL32( dst + 4, ReadLE32( tptr0 + 4 ) );
				DXV_SET_T1();
				break;
			default:
				break;
			}
		}
	}
	else
	{
		bCopyPrev = true;
	}

	if ( bCopyPrev )
	{
		DXV_WL32( dst, ReadLE32( dst - step ) );
		DXV_WL32( dst + 4, ReadLE32( dst - step + 4 ) );
		state--;
	}

#undef DXV_TAB
#undef DXV_BACKREF
#undef DXV_SET_T0
#undef DXV_SET_T1

	if ( gb.mbOverrun ) return false;
	dst += 8;
	return true;
}

// Single BC4 stream (8-byte blocks) - used for the YCG6 luma texture
static bool DXVHQDecompressYO( DXVStream& gb, BYTE* tex, uint32 texSize )
{
	BYTE*	tab0[256] = { 0 };
	BYTE*	tab1[256] = { 0 };
	uint32	opOffset = gb.LE32();
	uint32	opSize = gb.LE32();
	uint32	dataStart = gb.Tell();

	if ( ( gb.mbOverrun ) || ( opOffset < 8 ) || ( opOffset - 8 > gb.Left() ) ) return false;
	if ( ( opSize > texSize / 8 ) || ( texSize < 8 ) ) return false;

	gb.Skip( opOffset - 8 );
	std::vector<BYTE>	ops( opSize ? opSize : 1 );
	int		skip = DXVHQDecompressOpcodes( gb, ops.data(), opSize );
	if ( skip < 0 ) return false;
	gb.Seek( dataStart );

	BYTE*	dst = tex;
	DXV_WL32( dst, gb.LE32() );
	DXV_WL32( dst + 4, gb.LE32() );
	tab0[DXV_Hash16( DXV_RL16( dst ) )] = dst;
	tab1[DXV_Hash24( dst + 2 )] = dst + 2;
	dst += 8;

	uint32	oi = 0;
	int		state = 0;
	while ( dst < tex + texSize )
	{
		if ( !DXVHQDecompressCGO( gb, tex, texSize, ops.data(), oi, opSize, dst, state, tab0, tab1, 0 ) ) return false;
	}

	gb.Seek( dataStart + opOffset + (uint32)skip - 8 );
	return !gb.mbOverrun;
}

// Two interleaved BC4 streams (16-byte blocks) - used for the CoCg texture (and Y+Alpha in YG10)
static bool DXVHQDecompressCOCG( DXVStream& gb, BYTE* tex, uint32 texSize )
{
	BYTE*	tab0[256] = { 0 };
	BYTE*	tab1[256] = { 0 };
	BYTE*	tab2[256] = { 0 };
	BYTE*	tab3[256] = { 0 };
	uint32	opOffset = gb.LE32();
	uint32	opSize0 = gb.LE32();
	uint32	opSize1 = gb.LE32();
	uint32	dataStart = gb.Tell();

	if ( ( gb.mbOverrun ) || ( opOffset < 12 ) || ( opOffset - 12 > gb.Left() ) ) return false;
	if ( ( opSize0 > texSize / 8 ) || ( opSize1 > texSize / 8 ) || ( texSize < 16 ) ) return false;

	gb.Skip( opOffset - 12 );
	std::vector<BYTE>	ops0( opSize0 ? opSize0 : 1 );
	std::vector<BYTE>	ops1( opSize1 ? opSize1 : 1 );
	int		skip0 = DXVHQDecompressOpcodes( gb, ops0.data(), opSize0 );
	if ( skip0 < 0 ) return false;
	int		skip1 = DXVHQDecompressOpcodes( gb, ops1.data(), opSize1 );
	if ( skip1 < 0 ) return false;
	gb.Seek( dataStart );

	BYTE*	dst = tex;
	DXV_WL32( dst, gb.LE32() );
	DXV_WL32( dst + 4, gb.LE32() );
	DXV_WL32( dst + 8, gb.LE32() );
	DXV_WL32( dst + 12, gb.LE32() );
	tab0[DXV_Hash16( DXV_RL16( dst ) )] = dst;
	tab1[DXV_Hash24( dst + 2 )] = dst + 2;
	tab2[DXV_Hash16( DXV_RL16( dst + 8 ) )] = dst + 8;
	tab3[DXV_Hash24( dst + 10 )] = dst + 10;
	dst += 16;

	uint32	oi0 = 0, oi1 = 0;
	int		state0 = 0, state1 = 0;
	while ( dst + 10 < tex + texSize )
	{
		if ( !DXVHQDecompressCGO( gb, tex, texSize, ops0.data(), oi0, opSize0, dst, state0, tab0, tab1, 8 ) ) return false;
		if ( !DXVHQDecompressCGO( gb, tex, texSize, ops1.data(), oi1, opSize1, dst, state1, tab2, tab3, 8 ) ) return false;
	}

	gb.Seek( dataStart - 12 + opOffset + (uint32)skip0 + (uint32)skip1 );
	return !gb.mbOverrun;
}

static void BC4DecodeBlock( const BYTE* b, BYTE* out )
{
	uint32	e0 = b[0];
	uint32	e1 = b[1];
	BYTE	pal[8];
	pal[0] = (BYTE)e0;
	pal[1] = (BYTE)e1;
	if ( e0 > e1 )
	{
		for ( uint32 i = 1; i <= 6; i++ ) pal[i + 1] = (BYTE)( ( ( 7 - i ) * e0 + i * e1 ) / 7 );
	}
	else
	{
		for ( uint32 i = 1; i <= 4; i++ ) pal[i + 1] = (BYTE)( ( ( 5 - i ) * e0 + i * e1 ) / 5 );
		pal[6] = 0;
		pal[7] = 255;
	}
	u64		bits = 0;
	for ( int i = 0; i < 6; i++ ) bits |= (u64)b[2 + i] << ( 8 * i );
	for ( int i = 0; i < 16; i++ ) out[i] = pal[( bits >> ( 3 * i ) ) & 7];
}

//-------------------------------------------------------------------------------------------------
// DXVReader
//-------------------------------------------------------------------------------------------------
enum eDXVFrameFormat
{
	DXV_FRAME_UNKNOWN = 0,
	DXV_FRAME_DXT1,
	DXV_FRAME_DXT5,
	DXV_FRAME_YCOCG,		// HQ (YCG6)
	DXV_FRAME_YCOCG_ALPHA,	// HQ with alpha (YG10)
};

enum eDXVFrameCompression
{
	DXV_COMP_RAW = 0,
	DXV_COMP_LZF,
	DXV_COMP_DXV,			// Try DXV decompression (falls back to LZF)
};

struct DXVFrameHeader
{
	eDXVFrameFormat			format = DXV_FRAME_UNKNOWN;
	eDXVFrameCompression	compression = DXV_COMP_RAW;
	uint32					headerLen = 0;
};

static bool DXVParseFrameHeader( const BYTE* p, uint32 len, DXVFrameHeader* pHeader )
{
	if ( len < 4 ) return false;

	uint32	tag = ReadLE32( p );
	switch ( tag )
	{
	case DXV_TAG( 'D','X','T','1' ): pHeader->format = DXV_FRAME_DXT1; break;
	case DXV_TAG( 'D','X','T','5' ): pHeader->format = DXV_FRAME_DXT5; break;
	case DXV_TAG( 'Y','C','G','6' ): pHeader->format = DXV_FRAME_YCOCG; break;
	case DXV_TAG( 'Y','G','1','0' ): pHeader->format = DXV_FRAME_YCOCG_ALPHA; break;
	default:
		{
			// Old (DXV2) header - just size and type flags
			uint32	oldType = tag >> 24;
			int		versionMajor = (int)( oldType & 0x0F ) - 1;

			pHeader->compression = ( oldType & 0x80 ) ? DXV_COMP_RAW : DXV_COMP_LZF;
			if ( oldType & 0x40 )
			{
				pHeader->format = DXV_FRAME_DXT5;
			}
			else if ( ( oldType & 0x20 ) || ( versionMajor == 1 ) )
			{
				pHeader->format = DXV_FRAME_DXT1;
			}
			else
			{
				return false;
			}
			pHeader->headerLen = 4;
			return true;
		}
	}

	// New (DXV3) 12 byte header: tag, version major, version minor, compression, pad, size
	if ( len < 12 ) return false;
	// Encoder stores the texture raw when compression isn't advantageous (flag non-zero)
	pHeader->compression = ( p[6] != 0 ) ? DXV_COMP_RAW : DXV_COMP_DXV;
	pHeader->headerLen = 12;
	return true;
}

DXVReader::~DXVReader()
{
	Close();
}

void	DXVReader::Close()
{
	if ( mpFile )
	{
		fclose( mpFile );
		mpFile = NULL;
	}
	maFrames.clear();
	maReadBuffer.clear();
	mnNextFrame = 0;
}

bool	DXVReader::Open( const char* szFilename )
{
	Close();

	mpFile = fopen( szFilename, "rb" );
	if ( !mpFile ) return false;

	_fseeki64( mpFile, 0, SEEK_END );
	u64		ullFileSize = (u64)_ftelli64( mpFile );
	u64		ullPos = 0;
	std::vector<BYTE>	aMoov;

	// Find the moov atom at the top level
	while ( ullPos + 8 <= ullFileSize )
	{
		BYTE	hdr[16];
		_fseeki64( mpFile, (__int64)ullPos, SEEK_SET );
		if ( fread( hdr, 1, 8, mpFile ) != 8 ) break;

		u64		size = ReadBE32( hdr );
		uint32	type = ReadBE32( hdr + 4 );
		uint32	hdrLen = 8;
		if ( size == 1 )
		{
			if ( fread( hdr + 8, 1, 8, mpFile ) != 8 ) break;
			size = ReadBE64( hdr + 8 );
			hdrLen = 16;
		}
		else if ( size == 0 )
		{
			size = ullFileSize - ullPos;
		}
		if ( ( size < hdrLen ) || ( ullPos + size > ullFileSize ) ) break;

		if ( type == MOV_ATOM( 'm','o','o','v' ) )
		{
			aMoov.resize( (size_t)( size - hdrLen ) );
			if ( fread( aMoov.data(), 1, aMoov.size(), mpFile ) != aMoov.size() ) aMoov.clear();
			break;
		}
		ullPos += size;
	}

	if ( aMoov.empty() )
	{
		Close();
		return false;
	}

	std::vector<MovTrackInfo>	tracks;
	MovParseAtoms( aMoov.data(), aMoov.size(), NULL, tracks );

	const MovTrackInfo*	pTrack = NULL;
	for ( const MovTrackInfo& track : tracks )
	{
		if ( ( track.handlerType == MOV_ATOM( 'v','i','d','e' ) ) &&
			 ( ( track.codec == MOV_ATOM( 'D','X','D','3' ) ) || ( track.codec == MOV_ATOM( 'D','X','D','I' ) ) ) )
		{
			pTrack = &track;
			break;
		}
	}
	if ( ( !pTrack ) || ( pTrack->timescale == 0 ) || ( pTrack->width <= 0 ) || ( pTrack->height <= 0 ) )
	{
		Close();
		return false;
	}

	// Build the frame table from the chunk / sample tables
	uint32	sampleIndex = 0;
	size_t	stscIndex = 0;
	for ( size_t chunk = 0; ( chunk < pTrack->chunkOffsets.size() ) && ( sampleIndex < pTrack->sampleCount ); chunk++ )
	{
		while ( ( stscIndex + 1 < pTrack->stsc.size() ) &&
				( pTrack->stsc[stscIndex + 1].firstChunk <= chunk + 1 ) )
		{
			stscIndex++;
		}
		uint32	samplesInChunk = pTrack->stsc.empty() ? 1 : pTrack->stsc[stscIndex].samplesPerChunk;
		u64		offset = pTrack->chunkOffsets[chunk];

		for ( uint32 s = 0; ( s < samplesInChunk ) && ( sampleIndex < pTrack->sampleCount ); s++ )
		{
			Frame	frame;
			frame.ulSize = pTrack->fixedSampleSize ? pTrack->fixedSampleSize : pTrack->sampleSizes[sampleIndex];
			frame.ullFileOffset = offset;
			frame.ullTimestamp = 0;
			maFrames.push_back( frame );
			offset += frame.ulSize;
			sampleIndex++;
		}
	}

	u64		ullTime = 0;
	size_t	frameIndex = 0;
	for ( const MovTrackInfo::SttsEntry& entry : pTrack->stts )
	{
		for ( uint32 i = 0; ( i < entry.count ) && ( frameIndex < maFrames.size() ); i++ )
		{
			maFrames[frameIndex++].ullTimestamp = ( ullTime * 10000000ull ) / pTrack->timescale;
			ullTime += entry.delta;
		}
	}

	if ( maFrames.empty() )
	{
		Close();
		return false;
	}

	mnWidth = pTrack->width;
	mnHeight = pTrack->height;
	mnTexWidth = ( mnWidth + 15 ) & ~15;
	mnTexHeight = ( mnHeight + 15 ) & ~15;
	mfDurationSec = (float)( (double)pTrack->duration / (double)pTrack->timescale );

	// Peek the first frame to find the texture format
	BYTE	firstHeader[12];
	DXVFrameHeader	header;
	uint32	peekLen = ( maFrames[0].ulSize < 12 ) ? maFrames[0].ulSize : 12;
	_fseeki64( mpFile, (__int64)maFrames[0].ullFileOffset, SEEK_SET );
	if ( ( fread( firstHeader, 1, peekLen, mpFile ) != peekLen ) ||
		 ( !DXVParseFrameHeader( firstHeader, peekLen, &header ) ) )
	{
		SysDebugPrint( "DXV: Unrecognised frame header (%s)", szFilename );
		Close();
		return false;
	}

	mbHQ = ( header.format == DXV_FRAME_YCOCG ) || ( header.format == DXV_FRAME_YCOCG_ALPHA );
	mbHQAlpha = ( header.format == DXV_FRAME_YCOCG_ALPHA );
	mbDXT5 = ( header.format == DXV_FRAME_DXT5 );
	if ( mbHQ )
	{
		// Y (BC4, or Y+A interleaved) at full res, CoCg (two interleaved BC4 streams) at half res
		mulYTexSize = mbHQAlpha ? ( mnTexWidth * mnTexHeight ) : ( ( mnTexWidth * mnTexHeight ) / 2 );
		mulCTexSize = ( mnTexWidth * mnTexHeight ) / 4;
		maYTex.resize( mulYTexSize );
		maCTex.resize( mulCTexSize );
		maCgPlane.resize( ( mnTexWidth / 2 ) * ( mnTexHeight / 2 ) );
		maCoPlane.resize( ( mnTexWidth / 2 ) * ( mnTexHeight / 2 ) );
		mulTexSize = mnTexWidth * mnTexHeight * 4;
	}
	else
	{
		mulTexSize = mbDXT5 ? ( mnTexWidth * mnTexHeight ) : ( ( mnTexWidth * mnTexHeight ) / 2 );
	}
	mnNextFrame = 0;

	SysDebugPrint( "DXV: Opened %s (%dx%d, %s, %d frames)", szFilename, mnWidth, mnHeight,
					mbHQ ? ( mbHQAlpha ? "HQ+Alpha" : "HQ" ) : ( mbDXT5 ? "DXT5" : "DXT1" ), (int)maFrames.size() );
	return true;
}

void	DXVReader::ConvertHQToBGRA( BYTE* pDest )
{
	int		tw = mnTexWidth;
	int		th = mnTexHeight;
	int		cw = tw / 2;
	int		ch = th / 2;
	BYTE	block0[16];
	BYTE	block1[16];

	// Chroma planes (first half of each 16-byte block = Cg, second = Co)
	int		cbw = cw / 4;
	for ( int by = 0; by < ch / 4; by++ )
	{
		for ( int bx = 0; bx < cbw; bx++ )
		{
			const BYTE*	blk = maCTex.data() + ( by * cbw + bx ) * 16;
			BC4DecodeBlock( blk, block0 );
			BC4DecodeBlock( blk + 8, block1 );
			for ( int py = 0; py < 4; py++ )
			{
				BYTE*	pCg = maCgPlane.data() + ( by * 4 + py ) * cw + bx * 4;
				BYTE*	pCo = maCoPlane.data() + ( by * 4 + py ) * cw + bx * 4;
				memcpy( pCg, block0 + py * 4, 4 );
				memcpy( pCo, block1 + py * 4, 4 );
			}
		}
	}

	// Luma (+alpha) and YCoCg -> BGRA
	int		ybw = tw / 4;
	int		yStride = mbHQAlpha ? 16 : 8;
	for ( int by = 0; by < th / 4; by++ )
	{
		for ( int bx = 0; bx < ybw; bx++ )
		{
			const BYTE*	blk = maYTex.data() + ( by * ybw + bx ) * yStride;
			BC4DecodeBlock( blk, block0 );
			if ( mbHQAlpha ) BC4DecodeBlock( blk + 8, block1 );

			for ( int py = 0; py < 4; py++ )
			{
				int		y = by * 4 + py;
				BYTE*	pOut = pDest + ( y * tw + bx * 4 ) * 4;
				const BYTE*	pCg = maCgPlane.data() + ( y / 2 ) * cw;
				const BYTE*	pCo = maCoPlane.data() + ( y / 2 ) * cw;
				for ( int px = 0; px < 4; px++ )
				{
					int		x = bx * 4 + px;
					int		Y = block0[py * 4 + px];
					int		cg = (int)pCg[x / 2] - 128;
					int		co = (int)pCo[x / 2] - 128;
					int		r = Y + co - cg;
					int		g = Y + cg;
					int		b = Y - co - cg;
					pOut[0] = (BYTE)( ( b < 0 ) ? 0 : ( ( b > 255 ) ? 255 : b ) );
					pOut[1] = (BYTE)( ( g < 0 ) ? 0 : ( ( g > 255 ) ? 255 : g ) );
					pOut[2] = (BYTE)( ( r < 0 ) ? 0 : ( ( r > 255 ) ? 255 : r ) );
					pOut[3] = mbHQAlpha ? block1[py * 4 + px] : 255;
					pOut += 4;
				}
			}
		}
	}
}

bool	DXVReader::DecodeFrame( const BYTE* pSrc, uint32 ulSrcLen, BYTE* pDest )
{
	DXVFrameHeader	header;
	if ( !DXVParseFrameHeader( pSrc, ulSrcLen, &header ) ) return false;
	if ( header.format == DXV_FRAME_UNKNOWN ) return false;

	const BYTE*	pPayload = pSrc + header.headerLen;
	uint32		ulPayloadLen = ulSrcLen - header.headerLen;

	if ( mbHQ )
	{
		if ( header.format != ( mbHQAlpha ? DXV_FRAME_YCOCG_ALPHA : DXV_FRAME_YCOCG ) ) return false;

		bool	bOK = false;
		if ( ( header.compression == DXV_COMP_RAW ) && ( ulPayloadLen >= mulYTexSize + mulCTexSize ) )
		{
			memcpy( maYTex.data(), pPayload, mulYTexSize );
			memcpy( maCTex.data(), pPayload + mulYTexSize, mulCTexSize );
			bOK = true;
		}
		else
		{
			DXVStream	gb( pPayload, ulPayloadLen );
			bOK = mbHQAlpha ? DXVHQDecompressCOCG( gb, maYTex.data(), mulYTexSize )
							: DXVHQDecompressYO( gb, maYTex.data(), mulYTexSize );
			bOK = bOK && DXVHQDecompressCOCG( gb, maCTex.data(), mulCTexSize );
		}
		if ( !bOK ) return false;

		ConvertHQToBGRA( pDest );
		return true;
	}

	if ( ( header.format == DXV_FRAME_YCOCG ) || ( header.format == DXV_FRAME_YCOCG_ALPHA ) ) return false;
	if ( ( header.format == DXV_FRAME_DXT5 ) != mbDXT5 ) return false;

	if ( ( header.compression == DXV_COMP_RAW ) && ( ulPayloadLen >= mulTexSize ) )
	{
		memcpy( pDest, pPayload, mulTexSize );
		return true;
	}
	if ( header.compression == DXV_COMP_LZF )
	{
		return LZFDecompress( pPayload, ulPayloadLen, pDest, mulTexSize );
	}

	bool	bOK = mbDXT5 ? DXVDecompressDXT5( pPayload, ulPayloadLen, pDest, mulTexSize )
						 : DXVDecompressDXT1( pPayload, ulPayloadLen, pDest, mulTexSize );
	if ( !bOK )
	{
		bOK = LZFDecompress( pPayload, ulPayloadLen, pDest, mulTexSize );
	}
	return bOK;
}

HRESULT	DXVReader::ReadSample( DWORD* pFlags, LONGLONG* pTimestamp, IMFSample** ppSample )
{
	*pFlags = 0;
	*pTimestamp = 0;
	*ppSample = NULL;

	if ( !mpFile ) return E_FAIL;

	while ( mnNextFrame < maFrames.size() )
	{
		const Frame&	frame = maFrames[mnNextFrame++];

		if ( maReadBuffer.size() < frame.ulSize ) maReadBuffer.resize( frame.ulSize );
		_fseeki64( mpFile, (__int64)frame.ullFileOffset, SEEK_SET );
		if ( fread( maReadBuffer.data(), 1, frame.ulSize, mpFile ) != frame.ulSize )
		{
			continue;
		}

		IMFMediaBuffer*	pBuffer = NULL;
		IMFSample*		pSample = NULL;
		BYTE*			pbDest = NULL;

		if ( FAILED( MFCreateMemoryBuffer( mulTexSize, &pBuffer ) ) ) return E_OUTOFMEMORY;
		pBuffer->Lock( &pbDest, NULL, NULL );
		bool	bDecoded = DecodeFrame( maReadBuffer.data(), frame.ulSize, pbDest );
		pBuffer->Unlock();

		if ( bDecoded )
		{
			pBuffer->SetCurrentLength( mulTexSize );
			if ( SUCCEEDED( MFCreateSample( &pSample ) ) )
			{
				pSample->AddBuffer( pBuffer );
				pSample->SetSampleTime( (LONGLONG)frame.ullTimestamp );
				*ppSample = pSample;
				*pTimestamp = (LONGLONG)frame.ullTimestamp;
			}
			SAFE_RELEASE( pBuffer );
			return ( *ppSample ) ? S_OK : E_OUTOFMEMORY;
		}

		SysDebugPrint( "DXV: Failed to decode frame %d", (int)( mnNextFrame - 1 ) );
		SAFE_RELEASE( pBuffer );
	}

	*pFlags = MF_SOURCE_READERF_ENDOFSTREAM;
	return S_OK;
}

bool	DXVReader::Seek( u64 ullTimestamp )
{
	if ( maFrames.empty() ) return false;

	size_t	nFrame = 0;
	while ( ( nFrame + 1 < maFrames.size() ) &&
			( maFrames[nFrame + 1].ullTimestamp <= ullTimestamp ) )
	{
		nFrame++;
	}
	mnNextFrame = nFrame;
	return true;
}
