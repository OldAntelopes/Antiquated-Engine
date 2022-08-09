#ifndef NETWORKING_RING_BUFFER_H
#define NETWORKING_RING_BUFFER_H

#include "Networking.h"


class CRingBuffer
{
public:
	CRingBuffer() 
	{
		mnBufferSize = 0;
		mpbBufferMem = NULL;
		mnWritePos = 0;
		mnReadPos = 0;
	}
	~CRingBuffer() 
	{
		if ( mpbBufferMem )
		{
			SystemFree( mpbBufferMem );
			mpbBufferMem = NULL;
		}
	}

	void	Init( int nBufferSize )
	{
		mnBufferSize = nBufferSize;
		mpbBufferMem = (byte*)SystemMalloc( nBufferSize );
	}

	int	AddToBuffer( byte* szData, int nLen )
	{
	int		nBufferRemaining; 
	int		nStartBufferRemaining;
	int		nAmountWritten = 0;

		nBufferRemaining = mnBufferSize - mnWritePos;
		if ( nBufferRemaining >= nLen )
		{
			memcpy( mpbBufferMem + mnWritePos, szData, nLen );
			mnWritePos += nLen;
			nAmountWritten += nLen;
		}
		else
		{
			nStartBufferRemaining = mnWritePos;
			memcpy( mpbBufferMem + mnWritePos, szData, nBufferRemaining );
			nAmountWritten += nBufferRemaining;
			nLen -= nBufferRemaining;
			mnWritePos = 0;
			if ( nLen > 0 )
			{
				if ( nLen <= nStartBufferRemaining)
				{
					memcpy( mpbBufferMem + mnWritePos, szData, nLen );
					mnWritePos += nLen;
					nAmountWritten += nLen;
				}
				else
				{	// Buffer not big enough for a single sample?! oops..
				int		nBreak = 0;
					nBreak++;
					NetworkingUserDebugPrint( 0,"Buffer not big enough for a single sample");
				}
			}
			else if ( nLen < 0 )
			{	
			int		nBreak = 0;
				nBreak++;
				NetworkingUserDebugPrint( 0,"Add to buffer error, neg len after first copy", 0 );
				// um..
			}
		}
		return( nAmountWritten );
	}

	int		GetBytesAvailableToRead( void )
	{
		if ( mnWritePos > mnReadPos )
		{
			return( mnWritePos - mnReadPos );
		}
		else
		{
			return( (mnBufferSize - mnReadPos) + mnWritePos );
		}
	}

	int		ReadFromBuffer( byte* szData, int nLen )
	{
	int		nReadChunkSize;
	int		nAmountRead = 0;

		if ( mnWritePos >= mnReadPos )
		{
			nReadChunkSize = mnWritePos - mnReadPos;
			if ( nReadChunkSize < nLen )
			{
				nLen = nReadChunkSize;
			}
			if ( nLen > 0 )
			{
				memcpy( szData, mpbBufferMem + mnReadPos, nLen );
				mnReadPos += nLen;
				nAmountRead += nLen;
			}
		}
		else
		{
			nReadChunkSize = mnBufferSize - mnReadPos;
			if ( nReadChunkSize > nLen )
			{
				memcpy( szData, mpbBufferMem + mnReadPos, nLen );
				mnReadPos += nLen;
				nAmountRead += nLen;
			}
			else
			{
				memcpy( szData, mpbBufferMem + mnReadPos, nReadChunkSize );
				nAmountRead += nReadChunkSize;
				mnReadPos = 0;
				nLen -= nReadChunkSize;
				if ( nLen > mnWritePos )
				{
					nLen = mnWritePos;
				}
				if ( nLen > 0 )
				{
					memcpy( szData, mpbBufferMem + mnReadPos, nLen );
					mnReadPos += nLen;
					nAmountRead += nLen;
				}
			}
		}
		return( nAmountRead );
	}
private:
	int		mnBufferSize;
	byte*	mpbBufferMem;
	int		mnWritePos;
	int		mnReadPos;
};


#endif
