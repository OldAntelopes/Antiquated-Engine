#ifndef GAMECOMMON_MULTI_VERTEX_BUFFERS_H
#define GAMECOMMON_MULTI_VERTEX_BUFFERS_H

class MultiVertexBuffers
{
public:

	MultiVertexBuffers()
	{
		mnNumVertexBuffers = 0;
	}

	BOOL		IsInitialised( void ) { if ( mnNumVertexBuffers == 0 ) return( FALSE ); return( TRUE ); }
	BOOL		IsLocked( void ) { if ( mpulColStream == NULL ) return( FALSE ); return( TRUE ); }

	void		Init( int nNumBuffers, int nBufferSize, const char* szTrackingName )
	{
		if ( mnNumVertexBuffers == 0 )
		{
		int		nLoop;

			mpnVertexBuffers = (int*)( malloc( nNumBuffers * sizeof(int) ) );
			
			for( nLoop = 0; nLoop < nNumBuffers; nLoop++ )
			{
				mpnVertexBuffers[nLoop] = EngineCreateVertexBuffer( nBufferSize, 0, szTrackingName );
			}
			mnCurrentVertexBuffer = 0;
			mnCurrentPos = 0;
			mnVertexBufferSize = nBufferSize;
			mnNumVertexBuffers = nNumBuffers;
		}
	}

	void		Shutdown( void )
	{
	int		nLoop;

		if ( mpnVertexBuffers )
		{
			for( nLoop = 0; nLoop < mnNumVertexBuffers; nLoop++ )
			{
				EngineVertexBufferFree( mpnVertexBuffers[nLoop] );
			}
			free( mpnVertexBuffers );
			mpnVertexBuffers = NULL;
			mnNumVertexBuffers = 0;
		}
	}

	void		AddVert( void )
	{
		mpxPosStream = (VECT*)( ( (byte*)mpxPosStream) + mnPosStride );
		mpulColStream = (ulong*)( ( (byte*)mpulColStream) + mnColStride );
		mpfUVStream = (float*)( ( (byte*)mpfUVStream) + mnUVStride );
		mnCurrentPos++;
	}

	void		SetNextVertex( const VECT* pxPos, const VECT* pxNormal, float fU, float fV, ulong ulCol )
	{
		*mpulColStream = ulCol;
		*mpxPosStream = *pxPos;
		mpfUVStream[0] = fU;
		mpfUVStream[1] = fV;

		// todo - normals..
		AddVert();
	}

	void		Lock( void )
	{
		if ( mpnVertexBuffers )
		{
			mpulColStream = EngineVertexBufferLockColourStream( mpnVertexBuffers[mnCurrentVertexBuffer], &mnColStride );
			mpxPosStream = EngineVertexBufferLockPositionStream( mpnVertexBuffers[mnCurrentVertexBuffer], &mnPosStride );
			mpfUVStream = EngineVertexBufferLockUVStream( mpnVertexBuffers[mnCurrentVertexBuffer], &mnUVStride );
			mnCurrentPos = 0;
		}
	}

	void		FlushWhenFull( int nNumVerticesRequired, BOOL bLockNextBuffer )
	{
		if ( ( nNumVerticesRequired == 0 ) ||
			 ( mnCurrentPos >= mnVertexBufferSize - (nNumVerticesRequired+1) ) )
		{
			if ( mnCurrentPos > 0 )
			{
				EngineVertexBufferAddVertsUsed( mpnVertexBuffers[mnCurrentVertexBuffer], mnCurrentPos );
				EngineVertexBufferRender( mpnVertexBuffers[mnCurrentVertexBuffer], TRIANGLE_LIST );
				EngineVertexBufferReset( mpnVertexBuffers[mnCurrentVertexBuffer] );
				mnCurrentPos = 0;

				mpulColStream = NULL;
				if ( bLockNextBuffer )
				{
					mnCurrentVertexBuffer++;
					mnCurrentVertexBuffer %= mnNumVertexBuffers;
					Lock();
				}
			}
		}

	}

	ulong*		mpulColStream;
	VECT*		mpxPosStream;
	float*		mpfUVStream;

	int			mnColStride;
	int			mnPosStride;
	int			mnUVStride;

	int			mnCurrentPos;
	int			mnVertexBufferSize;

	int*		mpnVertexBuffers;
	int			mnCurrentVertexBuffer;
	int			mnNumVertexBuffers;

};


#endif // #ifndef GAMECOMMON_MULTI_VERTEX_BUFFERS_H
