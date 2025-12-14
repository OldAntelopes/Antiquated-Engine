//---------------------------------------------------------------
// EngineDX.cpp
//
//  This file contains the DirectX implementations of the functions defined in Engine.h
//
//---------------------------------------------------------------
#include "EngineDX.h"
#include "../../../Include/DirectX/d3dx9shader.h"

#include <StandardDef.h>
#include <Rendering.h>
#include <Interface.h>
#include <Engine.h>

#include "../EngineCamera.h"
#include "../ModelMaterialData.h"
#include "ShadowMapDX.h"
#include "TextureManagerDX.h"
#include "ShadowVolumeDX.h"

//-------------------------------------------------------------------------------------------------
LPGRAPHICS             g_pD3D       = NULL; // Used to create the D3DDevice
LPGRAPHICSDEVICE       g_pd3dDevice = NULL; // Our rendering device

LPGRAPHICSDEVICE       mpEngineDevice = NULL;
	
#define D3DFVF_SHADOWVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
#define D3DFVF_FLATVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define D3DFVF_LANDRENDERVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2)

// TODO - Get rid of this and make stuff dynamic
#define	MAX_ENGINE_INDEX_BUFFERS		256

int			msnNumberOfVertexBuffersCreated = 0;

class EngineVertBuffContainer
{
public:
	EngineVertBuffContainer()
	{
		pxVertexBuffer = NULL;
		pxLockedBuffer = NULL;
		nBufferPos = 0;
		nBufferSize = 0;
		m_nHandle = -1;
		mpxNext = NULL;
		mszTrackingName = NULL;
	}

	~EngineVertBuffContainer()
	{
		SAFE_FREE( mszTrackingName );
	}

	IGRAPHICSVERTEXBUFFER*		pxVertexBuffer;
	CUSTOMVERTEX*				pxLockedBuffer;
	int							nBufferPos;
	int							nBufferSize;
	char*						mszTrackingName;
	int							m_nHandle;
	EngineVertBuffContainer*	mpxNext;
};

class EngineIndexBuffContainer
{
public:
	EngineIndexBuffContainer()
	{
		pxIndexBuffer = NULL;
		puwLockedBuffer = NULL;
		m_nHandle = -1;
		mpxNext = NULL;
	}

	IGRAPHICSINDEXBUFFER*		pxIndexBuffer;
	ushort*						puwLockedBuffer;
	
	int							m_nHandle;
	EngineIndexBuffContainer*	mpxNext;
};

typedef struct
{
	IGRAPHICSINDEXBUFFER*		pxIndexBuffer;
	ushort*						puwLockedBuffer;

} ENGINE_INDEX_BUFF;


ENGINE_INDEX_BUFF			maxEngineIndexBuffers[MAX_ENGINE_INDEX_BUFFERS] = { { NULL, NULL } };

EngineVertBuffContainer*	m_spEngineVertBuffList = NULL;
int							m_snNextVertBufferHandle = 1;
EngineIndexBuffContainer*	m_spEngineIndexBuffList = NULL;
int							m_snNextIndexBufferHandle = 1;
BOOL						m_sbUseAnisotropicFiltering = TRUE;

int		mnEngineStateAlphaTest = NOTFOUND;

/*
typedef struct 
{
    VECT	position;
    VECT	normal; 
    unsigned long   color;
    float	tu;
	float	tv;
} ENGINEBUFFERVERTEX;
*/




EngineVertBuffContainer*		EngineVertexBufferGetContainer( int hVertBuffer )
{
EngineVertBuffContainer*	pVertBuff = m_spEngineVertBuffList;
		
	// Could do with optimising this - shouldnt have to walk a list every time
	while ( pVertBuff )
	{
		if( pVertBuff->m_nHandle == hVertBuffer )
		{
			return( pVertBuff );
		}
		pVertBuff = pVertBuff->mpxNext;
	}
	return( NULL );
}


EngineIndexBuffContainer*		EngineIndexBufferGetContainer( int hIndexBuffer )
{
EngineIndexBuffContainer*	pIndexBuff = m_spEngineIndexBuffList;
		
	while ( pIndexBuff )
	{
		if( pIndexBuff->m_nHandle == hIndexBuffer )
		{
			return( pIndexBuff );
		}
		pIndexBuff = pIndexBuff->mpxNext;
	}
	return( NULL );
}


INDEX_BUFFER_HANDLE		EngineCreateIndexBuffer( int nMaxIndices, int nType )
{
EngineIndexBuffContainer*		pIndexBuffContainer = new EngineIndexBuffContainer;

	if ( pIndexBuffContainer )
	{
	    if( FAILED( mpEngineDevice->CreateIndexBuffer( nMaxIndices * sizeof(ushort),
	                                              D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
		                                          D3DPOOL_DEFAULT, &pIndexBuffContainer->pxIndexBuffer, NULL ) ) )
		{
			delete pIndexBuffContainer;
			return( NOTFOUND );
		}

		int		newHandle = m_snNextIndexBufferHandle++;
			
		pIndexBuffContainer->m_nHandle = newHandle;
		pIndexBuffContainer->mpxNext = m_spEngineIndexBuffList;
		m_spEngineIndexBuffList = pIndexBuffContainer;
		return( newHandle );
	}
	return( NOTFOUND );
}

ushort*			EngineIndexBufferLock( INDEX_BUFFER_HANDLE hIndexBuffer, int flags )
{
EngineIndexBuffContainer*		pIndexBuffContainer = EngineIndexBufferGetContainer( hIndexBuffer );

	if ( pIndexBuffContainer )
	{
		if ( pIndexBuffContainer->pxIndexBuffer != NULL )
		{
			pIndexBuffContainer->pxIndexBuffer->Lock( 0, 0, (void**)&pIndexBuffContainer->puwLockedBuffer, 0 );

			return( pIndexBuffContainer->puwLockedBuffer );
		}
	}
	return( NULL );
}

void						EngineIndexBufferUnlock( INDEX_BUFFER_HANDLE hIndexBuffer )
{
EngineIndexBuffContainer*		pIndexBuffContainer = EngineIndexBufferGetContainer( hIndexBuffer );

	if ( pIndexBuffContainer )
	{
		if ( pIndexBuffContainer->pxIndexBuffer != NULL )
		{
			pIndexBuffContainer->pxIndexBuffer->Unlock();
		
		}
	}
}

BOOL	EngineIndexBufferRender( INDEX_BUFFER_HANDLE hIndexBuffer, VERTEX_BUFFER_HANDLE hVertexBuffer, int nNumPolys, int flags )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( hVertexBuffer );
EngineIndexBuffContainer*		pIndexBuffContainer = EngineIndexBufferGetContainer( hIndexBuffer );

	if ( ( pIndexBuffContainer ) &&
		 ( pVertBuffContainer ) )
	{
		int		nNumVerts = pVertBuffContainer->nBufferSize;

		mpEngineDevice->SetIndices( pIndexBuffContainer->pxIndexBuffer );
		mpEngineDevice->SetStreamSource( 0, pVertBuffContainer->pxVertexBuffer, 0, sizeof(CUSTOMVERTEX) );

	/*
		mpEngineDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, 0, nNumVerts, 0, nNumPolys, 
  [in] const void             *pIndexData,
  [in]       D3DFORMAT        IndexDataFormat,
  [in] const void             *pVertexStreamZeroData,
  [in]       UINT             VertexStreamZeroStride
);
*/
		mpEngineDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, nNumVerts, 0, nNumPolys );   
		return( TRUE );
	}
	return( FALSE );

}

void			EngineIndexBufferFree( INDEX_BUFFER_HANDLE hIndexBuffer )
{
	if ( hIndexBuffer != NOTFOUND )
	{
	EngineIndexBuffContainer*		pIndexBuffContainer = EngineIndexBufferGetContainer( hIndexBuffer );

		if ( pIndexBuffContainer )
		{
			if ( pIndexBuffContainer->pxIndexBuffer != NULL )
			{
				pIndexBuffContainer->pxIndexBuffer->Release();
				pIndexBuffContainer->pxIndexBuffer = NULL;
			}
		}

		// Now remove indexbuffercontainer from list
		EngineIndexBuffContainer*		pContainers = m_spEngineIndexBuffList;
		EngineIndexBuffContainer*		pLast = NULL;

		while( pContainers )
		{
			if ( pContainers == pIndexBuffContainer )
			{
				if ( pLast == NULL )
				{
					m_spEngineIndexBuffList = pContainers->mpxNext;
				}
				else
				{
					pLast->mpxNext = pContainers->mpxNext;
				}
				delete pContainers;
				return;
			}
			else
			{ 
				pLast = pContainers;
			}
			pContainers = pContainers->mpxNext;
		}

	}
}


//---------------------------------------------------------------------------------
VERTEX_BUFFER_HANDLE		EngineCreateVertexBuffer( int nMaxVertices, int nFlags, const char* szTrackingName )
{
EngineVertBuffContainer*		pVertBuffContainer = new EngineVertBuffContainer;

	if ( ( pVertBuffContainer ) &&
		 ( mpEngineDevice ) )
	{
	int		newHandle = m_snNextVertBufferHandle++;
	uint32	ulUsageFlags = 0;//D3DUSAGE_WRITEONLY;

		pVertBuffContainer->m_nHandle = newHandle;
		pVertBuffContainer->mszTrackingName = (char*)( malloc( strlen( szTrackingName ) + 1 ));
		if ( pVertBuffContainer->mszTrackingName )
		{
			strcpy( pVertBuffContainer->mszTrackingName, szTrackingName );
		}
		pVertBuffContainer->mpxNext = m_spEngineVertBuffList;
		m_spEngineVertBuffList = pVertBuffContainer;

		pVertBuffContainer->nBufferSize = 0;
		if ( nFlags == 0 )
		{
			ulUsageFlags |= D3DUSAGE_DYNAMIC;
		}
		else if ( nFlags == 2 )
		{
			ulUsageFlags = D3DUSAGE_WRITEONLY;
		}

	    if( !FAILED( mpEngineDevice->CreateVertexBuffer( (nMaxVertices+2) * sizeof(CUSTOMVERTEX),
														ulUsageFlags, D3DFVF_CUSTOMVERTEX,
														D3DPOOL_DEFAULT, &pVertBuffContainer->pxVertexBuffer, NULL ) ) )
	    {
			pVertBuffContainer->nBufferSize = nMaxVertices;
			msnNumberOfVertexBuffersCreated++;
			return( newHandle );
		}
	}
	return( NOTFOUND );
}

void		EngineVertexBufferReset( VERTEX_BUFFER_HANDLE  nHandle )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( nHandle );

	if ( pVertBuffContainer )
	{
		if ( pVertBuffContainer->pxVertexBuffer )
		{
			pVertBuffContainer->nBufferPos = 0;
		}
	}
}

uint32*	EngineVertexBufferLockColourStream( VERTEX_BUFFER_HANDLE handle , int* pnStride )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( handle );
uint32*		pulColBuff;

	if ( pVertBuffContainer )
	{
		if ( pVertBuffContainer->pxVertexBuffer )
		{
			if ( pVertBuffContainer->pxLockedBuffer == NULL )
			{
				if( FAILED( pVertBuffContainer->pxVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pVertBuffContainer->pxLockedBuffer, D3DLOCK_DISCARD)) )
				{
					pVertBuffContainer->pxLockedBuffer = NULL;
					return( NULL );
				}
			}

			pulColBuff = &pVertBuffContainer->pxLockedBuffer->color;
			*pnStride = sizeof( ENGINEBUFFERVERTEX );
			return( pulColBuff );
		}
	}
	return( NULL );
}


void		EngineVertexBufferUnlockColourStream( VERTEX_BUFFER_HANDLE handle )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( handle );
//uint32*		pulColBuff;

	if ( pVertBuffContainer )
	{
		if ( pVertBuffContainer->pxVertexBuffer )
		{
			if ( pVertBuffContainer->pxLockedBuffer != NULL )
			{
				pVertBuffContainer->pxVertexBuffer->Unlock();
				pVertBuffContainer->pxLockedBuffer = NULL;
			}
		}
	}

}

VECT*	EngineVertexBufferLockPositionStream( VERTEX_BUFFER_HANDLE handle, int* pnStride )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( handle );
VECT*		pxPosBuff;

	if ( pVertBuffContainer )
	{
		if ( pVertBuffContainer->pxVertexBuffer )
		{
			if ( pVertBuffContainer->pxLockedBuffer == NULL )
			{
				if( FAILED( pVertBuffContainer->pxVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pVertBuffContainer->pxLockedBuffer, D3DLOCK_DISCARD)) )
				{
					pVertBuffContainer->pxLockedBuffer = NULL;
					return( NULL );
				}
			}

			pxPosBuff = &pVertBuffContainer->pxLockedBuffer->position;
			*pnStride = sizeof( ENGINEBUFFERVERTEX );
			return( pxPosBuff );
		}
	}
	return( NULL );
}

VECT*	EngineVertexBufferLockNormalStream( VERTEX_BUFFER_HANDLE handle, int* pnStride )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( handle );
VECT*		pxPosBuff;

	if ( pVertBuffContainer )
	{
		if ( pVertBuffContainer->pxVertexBuffer )
		{
			if ( pVertBuffContainer->pxLockedBuffer == NULL )
			{
				if( FAILED( pVertBuffContainer->pxVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pVertBuffContainer->pxLockedBuffer, D3DLOCK_DISCARD)) )
				{
					pVertBuffContainer->pxLockedBuffer = NULL;
					return( NULL );
				}
			}

			pxPosBuff = &pVertBuffContainer->pxLockedBuffer->normal;
			*pnStride = sizeof( ENGINEBUFFERVERTEX );
			return( pxPosBuff );
		}
	}
	return( NULL );

}

float*	EngineVertexBufferLockUVStream( VERTEX_BUFFER_HANDLE handle, int* pnStride )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( handle );
float*		pfUVBuff;

	if ( pVertBuffContainer )
	{
		if ( pVertBuffContainer->pxVertexBuffer )
		{
			if ( pVertBuffContainer->pxLockedBuffer == NULL )
			{
				if( FAILED( pVertBuffContainer->pxVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pVertBuffContainer->pxLockedBuffer, D3DLOCK_DISCARD)) )
				{
					pVertBuffContainer->pxLockedBuffer = NULL;
					return( NULL );
				}
			}

			pfUVBuff = &pVertBuffContainer->pxLockedBuffer->tu;
			*pnStride = sizeof( ENGINEBUFFERVERTEX );
			return( pfUVBuff );
		}
	}
	return( NULL );
}

extern void		EngineVertexBufferUnlockPositionStream( VERTEX_BUFFER_HANDLE );
extern void		EngineVertexBufferUnlockNormalStream( VERTEX_BUFFER_HANDLE );
extern void		EngineVertexBufferUnlockUVStream( VERTEX_BUFFER_HANDLE );

BOOL		EngineVertexBufferLockAdd( VERTEX_BUFFER_HANDLE nHandle )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( nHandle );

	if ( pVertBuffContainer )
	{
#ifdef TUD11
		PANIC_IF( TRUE, "DX11 EngineVertexBufferLock TBI" );
#else
		if ( pVertBuffContainer->pxVertexBuffer )
		{
		int		nFlags = 0;

			if( FAILED( pVertBuffContainer->pxVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pVertBuffContainer->pxLockedBuffer, nFlags)) )
			{
				pVertBuffContainer->pxLockedBuffer = NULL;
				return( FALSE );
			}
			return( TRUE );
#endif
		}
	}
	return( FALSE );



}

BOOL		EngineVertexBufferLock( VERTEX_BUFFER_HANDLE nHandle, BOOL bClean )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( nHandle );

	if ( pVertBuffContainer )
	{
#ifdef TUD11
		PANIC_IF( TRUE, "DX11 EngineVertexBufferLock TBI" );
#else
		if ( pVertBuffContainer->pxVertexBuffer )
		{
		int		nFlags = 0;

			if ( bClean )
			{
				nFlags = D3DLOCK_DISCARD;
			}

			if( FAILED( pVertBuffContainer->pxVertexBuffer->Lock( 0, 0, (VERTEX_LOCKTYPE)&pVertBuffContainer->pxLockedBuffer, nFlags)) )
			{
				pVertBuffContainer->pxLockedBuffer = NULL;
				return( FALSE );
			}

			pVertBuffContainer->nBufferPos = 0;
			return( TRUE );
#endif
		}
	}
	return( FALSE );
}

ENGINEBUFFERVERTEX*		EngineVertexBufferGetBufferPointer( VERTEX_BUFFER_HANDLE nHandle, int nNumVertsRequired )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( nHandle );

	if ( ( pVertBuffContainer ) &&
		 ( pVertBuffContainer->pxLockedBuffer ) )
	{
	int		nPos = pVertBuffContainer->nBufferPos;

		if ( nPos <= pVertBuffContainer->nBufferSize - nNumVertsRequired )
		{
		CUSTOMVERTEX*	pVertBuffer;

			pVertBuffer = pVertBuffContainer->pxLockedBuffer + nPos;
			pVertBuffContainer->nBufferPos += nNumVertsRequired;
			return( (ENGINEBUFFERVERTEX*)( pVertBuffer ) );
		}
		else
		{
			return( NULL );
		}
	}
	return( NULL );
}


BOOL		EngineVertexBufferGet( VERTEX_BUFFER_HANDLE nHandle, int nVert, ENGINEBUFFERVERTEX* pxVert )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( nHandle );

	if ( pVertBuffContainer )
	{
		if ( pVertBuffContainer->pxLockedBuffer )
		{
		CUSTOMVERTEX*	pVertBuffer;
			pVertBuffer = pVertBuffContainer->pxLockedBuffer;
			pVertBuffer += nVert;
			*pxVert = *( (ENGINEBUFFERVERTEX*)pVertBuffer );
			return( TRUE );
		}
	}
	return( FALSE );
}

BOOL		EngineVertexBufferSet( VERTEX_BUFFER_HANDLE nHandle, int nVert, ENGINEBUFFERVERTEX* pxVert )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( nHandle );

	if ( pVertBuffContainer )
	{
		if ( pVertBuffContainer->pxLockedBuffer )
		{
		CUSTOMVERTEX*	pVertBuffer;
			pVertBuffer = pVertBuffContainer->pxLockedBuffer;
			pVertBuffer += nVert;
			*pVertBuffer = *( (CUSTOMVERTEX*)pxVert );
			if ( (nVert + 1) > pVertBuffContainer->nBufferPos )
			{
				pVertBuffContainer->nBufferPos = nVert + 1;
			}
			return( TRUE );
		}
	}
	return( FALSE );
}

void		EngineVertexBufferAddVertsUsed( VERTEX_BUFFER_HANDLE nHandle, int nNumVertsUsed )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( nHandle );

	if ( pVertBuffContainer )
	{
		pVertBuffContainer->nBufferPos += nNumVertsUsed;
	}
	
}

BOOL		EngineVertexBufferAdd( VERTEX_BUFFER_HANDLE nHandle, ENGINEBUFFERVERTEX* pxVertToAdd )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( nHandle );

	if ( pVertBuffContainer )
	{
		if ( pVertBuffContainer->pxLockedBuffer )
		{
			int		nPos = pVertBuffContainer->nBufferPos;

			if ( nPos < pVertBuffContainer->nBufferSize )
			{
			CUSTOMVERTEX*	pVertBuffer;
				pVertBuffer = pVertBuffContainer->pxLockedBuffer;
				pVertBuffer += nPos;
				*pVertBuffer = *( (CUSTOMVERTEX*)pxVertToAdd );
				pVertBuffContainer->nBufferPos++;
				return( TRUE );
			}
			else
			{
				nPos = nPos - 1;
			}
		}
	}
	return( FALSE );
}

BOOL		EngineVertexBufferUnlock( VERTEX_BUFFER_HANDLE nHandle )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( nHandle );

	if ( pVertBuffContainer )
	{
#ifdef TUD11
		PANIC_IF( TRUE, "DX11 EngineVertexBufferLock TBI" );
#else
		if ( ( pVertBuffContainer->pxVertexBuffer ) &&
			 ( pVertBuffContainer->pxLockedBuffer ) )
		{
			pVertBuffContainer->pxVertexBuffer->Unlock();
			pVertBuffContainer->pxLockedBuffer = NULL;
		}
#endif
	}
	return( FALSE );
}

BOOL	EngineVertexBufferCopyWithCol(  VERTEX_BUFFER_HANDLE hDestination,  VERTEX_BUFFER_HANDLE hSource, uint32 ulCol )
{
EngineVertBuffContainer*		pDestContainer = EngineVertexBufferGetContainer( hDestination );
EngineVertBuffContainer*		pSourceContainer = EngineVertexBufferGetContainer( hSource );
CUSTOMVERTEX*	pSourceVerts;
CUSTOMVERTEX*	pDestVerts;

	if ( ( pSourceContainer ) &&
		 ( pDestContainer ) ) 
	{
		pSourceVerts = pSourceContainer->pxLockedBuffer;
		pDestVerts = pDestContainer->pxLockedBuffer;

		if ( ( pSourceVerts ) &&
 			 ( pDestVerts ) )
		{
		int		nNumVertsToCopy = pSourceContainer->nBufferPos;
		
			if (  pDestContainer->nBufferPos + nNumVertsToCopy < pDestContainer->nBufferSize )
			{
			int		nLoop;

				pDestVerts += pDestContainer->nBufferPos;

				memcpy( pDestVerts, pSourceVerts, sizeof( CUSTOMVERTEX ) * nNumVertsToCopy );

				for( nLoop = 0; nLoop < nNumVertsToCopy; nLoop++ )
				{
					pDestVerts->color = ulCol;
					pDestVerts++;
				}
				pDestContainer->nBufferPos += nNumVertsToCopy; 
				return( TRUE );
			}
		}
	}
	return( FALSE );
}


BOOL	EngineVertexBufferCopy(  VERTEX_BUFFER_HANDLE hDestination,  VERTEX_BUFFER_HANDLE hSource )
{
EngineVertBuffContainer*		pDestContainer = EngineVertexBufferGetContainer( hDestination );
EngineVertBuffContainer*		pSourceContainer = EngineVertexBufferGetContainer( hSource );
CUSTOMVERTEX*	pSourceVerts;
CUSTOMVERTEX*	pDestVerts;

	if ( ( pSourceContainer ) &&
		 ( pDestContainer ) ) 
	{
		pSourceVerts = pSourceContainer->pxLockedBuffer;
		pDestVerts = pDestContainer->pxLockedBuffer;

		if ( ( pSourceVerts ) &&
 			 ( pDestVerts ) )
		{
		int		nNumVertsToCopy = pSourceContainer->nBufferPos;
		
			if (  pDestContainer->nBufferPos + nNumVertsToCopy < pDestContainer->nBufferSize )
			{
				pDestVerts += pDestContainer->nBufferPos;
				memcpy( pDestVerts, pSourceVerts, sizeof( CUSTOMVERTEX ) * nNumVertsToCopy );
				pDestContainer->nBufferPos += nNumVertsToCopy; 
				return( TRUE );
			}
		}
	}
	return( FALSE );
}


BOOL		EngineVertexBufferRender( VERTEX_BUFFER_HANDLE nHandle, ENGINEPRIMITIVE_TYPE nPrimType )
{
EngineVertBuffContainer*		pVertBuffContainer = EngineVertexBufferGetContainer( nHandle );

	if ( pVertBuffContainer )
	{
	int		nPrimsToDraw = pVertBuffContainer->nBufferPos;
	D3DPRIMITIVETYPE	primType = D3DPT_TRIANGLELIST;
	HRESULT		ret;

		if ( ( pVertBuffContainer->pxVertexBuffer ) &&
			 ( pVertBuffContainer->pxLockedBuffer != NULL ) )
		{
			pVertBuffContainer->pxVertexBuffer->Unlock();
			pVertBuffContainer->pxLockedBuffer = NULL;
		}
		switch( nPrimType )
		{
		case POINT_LIST:
			primType = D3DPT_POINTLIST;
			break;
		case LINE_LIST:
			primType = D3DPT_LINELIST;
			nPrimsToDraw /= 2;
			break;
		case TRIANGLE_LIST:
			nPrimsToDraw /= 3;
			break;
		}

		if ( nPrimsToDraw > 0 )
		{
			EngineSetVertexFormat( VERTEX_FORMAT_NORMAL );
			ret = mpEngineDevice->SetStreamSource( 0, pVertBuffContainer->pxVertexBuffer, 0, sizeof(CUSTOMVERTEX) );
			ret = mpEngineDevice->DrawPrimitive( primType, 0, nPrimsToDraw );
			return( TRUE );
		}
	}
	return( FALSE );
}

int		EngineGetNumVertexBuffersAllocated( void )
{
	return(	msnNumberOfVertexBuffersCreated );
}

void		EngineVertexBufferTrackingListAllocated( char* acErrorOut )
{
EngineVertBuffContainer*	pVertBuffContainer = m_spEngineVertBuffList;
char	acString[256];

	while ( pVertBuffContainer )
	{
		sprintf( acString, "-- %s\r\n", pVertBuffContainer->mszTrackingName );	
		strcat( acErrorOut, acString );
		pVertBuffContainer = pVertBuffContainer->mpxNext;
	}

}




void		EngineVertexBufferFree( VERTEX_BUFFER_HANDLE nHandle )
{
	if ( nHandle != NOTFOUND )
	{
	EngineVertBuffContainer*	pVertBuffContainer = m_spEngineVertBuffList;
	EngineVertBuffContainer*	pLast = NULL;
	EngineVertBuffContainer*	pNext = NULL;
		
		while ( pVertBuffContainer )
		{
			pNext = pVertBuffContainer->mpxNext;

			if( pVertBuffContainer->m_nHandle == nHandle )
			{
				if ( pVertBuffContainer->pxVertexBuffer )
				{
					pVertBuffContainer->pxVertexBuffer->Release();
					pVertBuffContainer->pxVertexBuffer = NULL;
				}	

				if ( pLast )
				{
					pLast->mpxNext = pNext;
				}
				else
				{
					m_spEngineVertBuffList = pNext;
				}
				delete pVertBuffContainer;
				msnNumberOfVertexBuffersCreated--;
				return;
			}
			else
			{
				pLast = pVertBuffContainer;
			}
			pVertBuffContainer = pNext;
		}
	}
}

//---------------------------------------------------------------------------------

LPGRAPHICSDEVICE	EngineGetDXDevice(void)
{
	return( mpEngineDevice );
}

int		mnLastSetVertexFormat = NOTFOUND;

void	EngineSetVertexFormat( int nVertexFormat )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetVertexFormat TBI" );
#else
 	if ( nVertexFormat != mnLastSetVertexFormat )
	{
		mnLastSetVertexFormat = nVertexFormat;

		switch( nVertexFormat )
		{
		default:
			mpEngineDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
			break;
		case VERTEX_FORMAT_NORMAL:
			mpEngineDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
			break;
		case VERTEX_FORMAT_2UVS:
			mpEngineDevice->SetFVF( D3DFVF_LANDRENDERVERTEX );
			break;
		case VERTEX_FORMAT_XYZ:
		    mpEngineDevice->SetFVF( D3DFVF_XYZ );
			break;
		case VERTEX_FORMAT_SHADOWVERTEX:
		    mpEngineDevice->SetFVF( D3DFVF_SHADOWVERTEX );
			break;
		case VERTEX_FORMAT_FLATVERTEX:
		    mpEngineDevice->SetFVF( D3DFVF_FLATVERTEX );
			break;
		}
	}
#endif
}

eSurfaceFormat		EngineDXGetSurfaceFormat( IGRAPHICSFORMAT format )
{
eSurfaceFormat	surfaceFormat;

#ifdef TUD11
	// TEMP - todoDX11!!
	surfaceFormat = SURFACEFORMAT_A8R8G8B8;
#else
	switch( format )
	{
//	case SURFACEFORMAT_UNKNOWN:
	case D3DFMT_UNKNOWN:
		surfaceFormat = SURFACEFORMAT_UNKNOWN;
		break;
	case D3DFMT_A8R8G8B8:
	default:
		surfaceFormat = SURFACEFORMAT_A8R8G8B8;
		break;
	case D3DFMT_X8R8G8B8:
		surfaceFormat = SURFACEFORMAT_X8R8G8B8;
		break;
	case D3DFMT_D24S8:
		surfaceFormat = SURFACEFORMAT_D24S8;
		break;
	case D3DFMT_R8G8B8:
		surfaceFormat = SURFACEFORMAT_X8R8G8B8;
		break;
	case D3DFMT_DXT1:
		surfaceFormat = SURFACEFORMAT_DXT1;
		break;
	case D3DFMT_DXT3:
		surfaceFormat = SURFACEFORMAT_DXT3;
		break;
	case D3DFMT_DXT5:
		surfaceFormat = SURFACEFORMAT_DXT5;
		break;
	case D3DFMT_A1R5G5B5:
		surfaceFormat = SURFACEFORMAT_A1R5G5B5;
		break;
	}
#endif
	return( surfaceFormat );
}

char	mszActivePixelShaderName[64] = "None";
char	mszActiveVertexShaderName[64] = "None";


void				EngineSetPixelShader( LPGRAPHICSPIXELSHADER pPixelShader, const char* szName )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineDXSetPixelShader TBI" );
#else
	mpEngineDevice->SetPixelShader( pPixelShader );
	if ( szName )
	{
		strcpy( mszActivePixelShaderName, szName );
	}
	else
	{
		strcpy( mszActivePixelShaderName, "None" );
	}
#endif
}

void				EngineSetVertexShader( LPGRAPHICSVERTEXSHADER pVertexShader, const char* szName )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineDXSetVertexShader TBI" );
#else
	mpEngineDevice->SetVertexShader( pVertexShader );
	if ( szName )
	{
		strcpy( mszActiveVertexShaderName, szName );
	}
	else
	{
		strcpy( mszActiveVertexShaderName, "None" );
	}
#endif
}


void		EngineSetMipBias( float fVal  )
{
	mpEngineDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, (DWORD)fVal);
}


IGRAPHICSFORMAT		EngineDXGetGraphicsFormat( eSurfaceFormat format )
{
IGRAPHICSFORMAT		dxFormat;

#ifdef TUD11
	// TEMP - todoDX11!!
	dxFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
#else
	switch( format )
	{
	case SURFACEFORMAT_UNKNOWN:
		dxFormat = D3DFMT_UNKNOWN;
		break;
	case SURFACEFORMAT_A8R8G8B8:
	default:
		dxFormat = D3DFMT_A8R8G8B8;
		break;
	case SURFACEFORMAT_X8R8G8B8:
		dxFormat = D3DFMT_X8R8G8B8;
		break;
	case SURFACEFORMAT_DXT1:
		dxFormat = D3DFMT_DXT1;
		break;
	case SURFACEFORMAT_DXT3:
		dxFormat = D3DFMT_DXT3;
		break;
	case SURFACEFORMAT_DXT5:
		dxFormat = D3DFMT_DXT5;
		break;
	case SURFACEFORMAT_D24S8:
		dxFormat = D3DFMT_D24S8;
		break;
	case SURFACEFORMAT_A1R5G5B5:
		dxFormat = D3DFMT_A1R5G5B5;
		break;
	}
#endif
	return( dxFormat );
}

D3DVIEWPORT9		mxRestoreViewport;

void		EngineSetViewport( int X, int Y, int W, int H )
{
D3DVIEWPORT9		xViewport;
	g_pd3dDevice->GetViewport( &mxRestoreViewport );
	xViewport = mxRestoreViewport;
	xViewport.X = X;
	xViewport.Y = Y;
	xViewport.Width = W;
	xViewport.Height = H;
	xViewport.MinZ = 0.0f;
	xViewport.MaxZ = 0.1f;

	g_pd3dDevice->SetViewport( &xViewport );
}

void		EngineRestoreViewport( void )
{
	g_pd3dDevice->SetViewport( &mxRestoreViewport );
}


void	EngineGetRayForScreenCoord( int nScreenX, int nScreenY, VECT* pxRayStart, VECT* pxRayDir )
{
D3DXMATRIX matProj;
D3DXMATRIX matView;
D3DXMATRIX matWorld;
D3DXMATRIX m;
D3DXVECTOR3	xVect;
//int		nMidX;
//int		nMidY;
POINT ptCursor;

	xVect.x = 0.0f;
	xVect.y = 0.0f;
	xVect.z = 0.0f;

	mpEngineDevice->GetTransform( D3DTS_PROJECTION, &matProj );
	mpEngineDevice->GetTransform( D3DTS_VIEW, &matView );

	ptCursor.x = nScreenX;
	ptCursor.y = nScreenY;

	// Compute the vector of the pick ray in screen space
	D3DXVECTOR3 v;
	v.x =  ( ( ( 1.0f * ptCursor.x ) / (InterfaceGetWidth()*0.5f) ) - 1 ) / matProj._11;
	v.y = -( ( ( 1.0f * ptCursor.y ) / (InterfaceGetHeight()*0.5f) ) - 1 ) / matProj._22;
	v.z =  1.0f;

	// Get the inverse view matrix
	D3DXMatrixInverse( &m, NULL, &matView );

	// Transform the screen space pick ray into 3D space
	pxRayDir->x  = v.x*m._11 + v.y*m._21 + v.z*m._31;
	pxRayDir->y  = v.x*m._12 + v.y*m._22 + v.z*m._32;
	pxRayDir->z  = v.x*m._13 + v.y*m._23 + v.z*m._33;
	pxRayStart->x = m._41;
	pxRayStart->y = m._42;
	pxRayStart->z = m._43;
}


void	EngineMatrixCreateScale( ENGINEMATRIX* pMat, float fScale )
{
	D3DXMatrixScaling( (D3DXMATRIX*)pMat, fScale, fScale, fScale );
}

void	EngineEnableTextureAddressClamp( int nFlag )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineEnableTextureAddressClamp TBI" );
#else
	if ( nFlag == 1 )
	{
	    mpEngineDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR );
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR );
	}
	else
	{
	    mpEngineDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	}
#endif
}


void	EngineEnableAlphaTest( int nFlag )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineEnableAlphaTest TBI" );
#else
	if ( mnEngineStateAlphaTest != nFlag )
	{
		mnEngineStateAlphaTest = nFlag;
		mpEngineDevice->SetRenderState( D3DRS_ALPHATESTENABLE, nFlag );
		
	}
#endif
}

// Lot of this stuff should be made into macros for optimisation purposes...
void	EngineEnableCulling( int nMode )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineEnableCulling TBI" );
#else

	if ( nMode == 0 )
	{
	    mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	}
	else if ( nMode == 2 )
	{
	    mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
	}
	else
	{
	    mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	}
#endif
}

void	EngineSetTextureDirect( int nTex, void* pTexture )
{
	mpEngineDevice->SetTexture( nTex, (LPGRAPHICSTEXTURE)pTexture );
}


void	EngineSetWorldMatrix( const ENGINEMATRIX* pxWorldMatrix )
{
ENGINEMATRIX	xIdentity;

	if ( pxWorldMatrix == NULL )
	{
		EngineMatrixIdentity( &xIdentity );
		pxWorldMatrix = &xIdentity;
	}

#ifdef TUD11
	// Note on DX11 we'd just need to make sure the transform is set in all the
	// appropriate constant buffers
	PANIC_IF( TRUE, "DX11 EngineSetWorldMatrix TBI" );
#else
	mpEngineDevice->SetTransform( D3DTS_WORLD, (D3DXMATRIX*)pxWorldMatrix );
#endif

	if ( ModelRenderingIsShadowPass() )
	{
		EngineShadowMapSetWorldMatrix( pxWorldMatrix );
	}
	else 
	{
		ModelMaterialsNormalShaderUpdateWorldTransform( pxWorldMatrix );
	}
}

void	EngineSetViewMatrix( const ENGINEMATRIX* pMat )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetViewMatrix TBI" );
#else
	mpEngineDevice->SetTransform( D3DTS_VIEW, (D3DXMATRIX*)pMat );
#endif
}

void		EngineSetProjectionMatrix( const ENGINEMATRIX* pMat )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetProjectionMatrix TBI" );
#else
	mpEngineDevice->SetTransform( D3DTS_PROJECTION, (D3DXMATRIX*)pMat );
#endif
}

void	EngineGetWorldMatrix( ENGINEMATRIX* pMat )
{
	mpEngineDevice->GetTransform( D3DTS_WORLD, pMat );
}

void	EngineGetProjectionMatrix( ENGINEMATRIX* pMat )
{
	mpEngineDevice->GetTransform( D3DTS_PROJECTION, pMat );
}

void	EngineGetViewMatrix( ENGINEMATRIX* pMat )
{
	mpEngineDevice->GetTransform( D3DTS_VIEW, pMat );
}

void		EngineActivateClippingPlane( BOOL bFlag, float fZHeight )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineActivateClippingPlane TBI" );
#else
	if ( bFlag )
	{
	D3DXPLANE waterPlane( 0.0f, 0.0f, 1.0f, -fZHeight );
	HRESULT	ret;

		mpEngineDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0 );
		ret = mpEngineDevice->SetClipPlane( 0, (float*)&waterPlane );
	}
	else
	{
		mpEngineDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, 0 );
	}
#endif
}


void	EngineSetShadeMode( int nFlag )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetShadeMode TBI" );
#else
	if ( nFlag == 1 )
	{
		mpEngineDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	}
	else
	{
		mpEngineDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	}
#endif
}

void	EngineSetZBias( int Value )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetZBias TBI" );
#else
	mpEngineDevice->SetRenderState(D3DRS_DEPTHBIAS, Value );
	ModelMaterialsNormalShaderUpdateZBias( Value * 0.01f );
#endif
}

BOOL		EngineCanUseAnisotropic( void )
{
	if ( InterfaceGetDeviceCaps( MAX_ANISTROPY ) > 0 )
	{
		return( m_sbUseAnisotropicFiltering );
	}
	return( FALSE );
}

void	EngineEnableAnisotropicFiltering( BOOL bFlag )
{
	m_sbUseAnisotropicFiltering = bFlag;
}

void	EngineTextureSetBestFiltering( int nChannel, int nMaxAnisotropy )
{
	if ( EngineCanUseAnisotropic() == TRUE )
	{
	int		nAnistropyLevels = InterfaceGetDeviceCaps( MAX_ANISTROPY );

		if ( nAnistropyLevels > nMaxAnisotropy )
		{
			nAnistropyLevels = nMaxAnisotropy;
		}
		mpEngineDevice->SetSamplerState( nChannel, D3DSAMP_MIPFILTER,  D3DTEXF_ANISOTROPIC );	
		mpEngineDevice->SetSamplerState( nChannel, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);	
		mpEngineDevice->SetSamplerState( nChannel, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		mpEngineDevice->SetSamplerState( nChannel, D3DSAMP_MAXANISOTROPY, nAnistropyLevels );
	}
	else
	{
		mpEngineDevice->SetSamplerState( nChannel, D3DSAMP_MIPFILTER,  D3DTEXF_LINEAR);	
		mpEngineDevice->SetSamplerState( nChannel, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);	
		mpEngineDevice->SetSamplerState( nChannel, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);	
	}
}

void	EngineSetTextureFiltering( int nMode )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetTextureFiltering TBI" );
#else
	switch( nMode )
	{
	case 0:
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER,  D3DTEXF_NONE );	
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);	
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);	
		break;
	case 1:
		EngineTextureSetBestFiltering( 0, 8 );
		break;
	case 2:
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER,  D3DTEXF_LINEAR);	
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);	
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);	
		break;
	case 3:
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER,  D3DTEXF_LINEAR);	
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_GAUSSIANQUAD);	
		mpEngineDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_GAUSSIANQUAD);	
		break;
	}
#endif
}

void		EngineSetMaterialColourSource( BOOL bFlag )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetMaterialColourSource TBI" );
#else
	if ( bFlag )
	{
		mpEngineDevice->SetRenderState(	D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
		mpEngineDevice->SetRenderState(	D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
	}
	else
	{
		mpEngineDevice->SetRenderState(	D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
		mpEngineDevice->SetRenderState(	D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2 );
	}
#endif
}

//--------------------------------------------------
// EngineSetBlendMode
//	Sets the type of semi-transparent blending
//--------------------------------------------------
void	EngineSetBlendMode( int nBlendMode )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineSetBlendMode TBI" );
#else
	switch( nBlendMode )
	{
	case BLEND_MODE_SRCALPHA_ADDITIVE:
		mpEngineDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		mpEngineDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	case BLEND_MODE_ALPHABLEND:
		mpEngineDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		mpEngineDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	case BLEND_MODE_COLOUR_SUBTRACTIVE:
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		break;
	case BLEND_MODE_ALPHA_SUBTRACTIVE:
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	case BLEND_MODE_COLOUR_BLEND:
		mpEngineDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		mpEngineDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		break;
	case BLEND_MODE_COLOUR_ADDITIVE:
		mpEngineDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		mpEngineDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;
	case BLEND_MODE_COLOUR_INVALPHA:
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	case BLEND_MODE_RAWCOLOUR_SUBTRACTIVE:
		mpEngineDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		mpEngineDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
		break;
	}
#endif
}

int		manEngineColourMode[4] = { NOTFOUND };

void	EngineResetColourMode( void )
{
	manEngineColourMode[0] = NOTFOUND;
}

//--------------------------------------------------
// EngineSetColourMode
//	Determines how the polys are coloured (for instance, by using the colour of
//  the texture modulated with the vertex colour)
//--------------------------------------------------
void	EngineSetColourMode( int nTexLayer, int nColourMode )
{
	if ( nColourMode != manEngineColourMode[nTexLayer] )
	{
		manEngineColourMode[nTexLayer] = nColourMode;

#ifdef TUD11
		PANIC_IF( TRUE, "DX11 EngineSetColourMode TBI" );
#else
		switch( nColourMode )
		{
		case COLOUR_MODE_TEXTURE_MODULATE:
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLOROP,   D3DTOP_MODULATE);
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
			break;
		case COLOUR_MODE_TEXTURE_ONLY:
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );	
			break;
		case COLOUR_MODE_DIFFUSE_ONLY:
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );	
			break;
		case COLOUR_MODE_TEXTURE_DIFFUSE_ALPHA:
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			break;
		case COLOUR_MODE_TEXTURE_MODULATE_NO_ALPHA_TEXTURE:
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLOROP,   D3DTOP_MODULATE);
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );	
			break;
		case COLOUR_MODE_DIFFUSE_ALPHA_TEXTURE:
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );	
			mpEngineDevice->SetTextureStageState( nTexLayer, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );	
			break;
		}
#endif
	}
}


int		mnEngineAlphaBlendState = NOTFOUND;
//--------------------------------------------------
// EngineEnableAlphaBlend
//	Enables or disables semi-transparent blending
//--------------------------------------------------
void	EngineEnableBlend( int nFlag )
{
	if ( nFlag != mnEngineAlphaBlendState )
	{
		mnEngineAlphaBlendState = nFlag;
		mpEngineDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, nFlag );
	}
}

int		mnEngineLightingFlag = TRUE;

void	EngineEnableLighting( int nFlag )
{
	if ( nFlag != mnEngineLightingFlag )
	{
		mpEngineDevice->SetRenderState( D3DRS_LIGHTING, nFlag );
		mnEngineLightingFlag = nFlag;

		ModelMaterialsEnableLighting( nFlag );
	}
}

int		mnEngineStateFog = NOTFOUND;

void	EngineEnableFog( int nFlag )
{
	if ( mnEngineStateFog != nFlag )
	{
		mnEngineStateFog = nFlag;
		if ( ModelRenderingIsShadowPass() == TRUE )
		{
			mpEngineDevice->SetRenderState( D3DRS_FOGENABLE, 0 );//nFlag );
			mnEngineStateFog = 0;
		}
		else
		{
			mpEngineDevice->SetRenderState( D3DRS_FOGENABLE, nFlag );
		}
	}
}

int		mnEngineCurrentZTest = NOTFOUND;

void	EngineEnableZTest( int nFlag )
{
	if ( mnEngineCurrentZTest != nFlag )
	{
		mnEngineCurrentZTest = nFlag;
	    mpEngineDevice->SetRenderState( D3DRS_ZENABLE, nFlag );
	}
}

int		mnCurrentSpecularFlag = 0;

void	EngineEnableSpecular( int nFlag )
{
	if ( mnCurrentSpecularFlag != nFlag)
	{
		mpEngineDevice->SetRenderState( D3DRS_SPECULARENABLE, nFlag );
		mnCurrentSpecularFlag = nFlag;
	}
}

int		mnEngineCurrentZWrite = NOTFOUND;

void	EngineEnableZWrite( int nFlag )
{
	if ( mnEngineCurrentZWrite != nFlag )
	{
		mnEngineCurrentZWrite = nFlag;
		mpEngineDevice->SetRenderState( D3DRS_ZWRITEENABLE, nFlag );
	}
}

void	EngineEnableWireframe( int nFlag )
{
	if ( nFlag == 1 )
	{
		mpEngineDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	}
	else
	{
		mpEngineDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}
}


void	EngineSetPointRenderSize( float fPointSize )
{
	mpEngineDevice->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE );
	mpEngineDevice->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&fPointSize));
}

void	EngineEnablePointFill( int nFlag )
{
	if ( nFlag == 1 )
	{
	float	fPointSize = 3.0f;
		mpEngineDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_POINT );
		mpEngineDevice->SetRenderState( D3DRS_POINTSIZE, *((DWORD*)&fPointSize ) );	 
	}
	else
	{
		mpEngineDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}
}

BOOL	EngineIsInitialised()
{
	if ( mpEngineDevice )
	{
		return( TRUE );
	}
	return( FALSE );
}


void	EngineSetMaterial( ENGINEMATERIAL* pxMaterial )
{
	ModelMaterialShaderSetMaterialProperties( (MATERIAL_COLOUR*)&pxMaterial->Diffuse, (MATERIAL_COLOUR*)&pxMaterial->Ambient, (MATERIAL_COLOUR*)&pxMaterial->Specular, (MATERIAL_COLOUR*)&pxMaterial->Emissive, pxMaterial->Power );

	mpEngineDevice->SetMaterial((GRAPHICSMATERIAL*)pxMaterial);
}

void		EngineGetMaterial( ENGINEMATERIAL* pxMaterialOut )
{
	mpEngineDevice->GetMaterial( (GRAPHICSMATERIAL*)pxMaterialOut );
}


void		EngineSetStandardMaterialWithAlpha( float fAlphaVal )
{
ENGINEMATERIAL xStandardMat;
	
	memset( &xStandardMat, 0, sizeof( xStandardMat ) );

	// Set the RGBA for diffuse reflection.
	xStandardMat.Diffuse.r = 1.0f;
	xStandardMat.Diffuse.g = 1.0f;
	xStandardMat.Diffuse.b = 1.0f;
	xStandardMat.Diffuse.a = fAlphaVal;
	
	// Set the RGBA for ambient reflection.
	xStandardMat.Ambient.r = 1.0f;
	xStandardMat.Ambient.g = 1.0f;
	xStandardMat.Ambient.b = 1.0f;
	xStandardMat.Ambient.a = fAlphaVal;

	EngineSetMaterial(&xStandardMat);
}


void		EngineSetStandardMaterialWithSpecular( float fPower, float fSpecularBrightness )
{
ENGINEMATERIAL xStandardMat;
	 
	// Set the RGBA for diffuse reflection.
	xStandardMat.Diffuse.r = 1.0f;
	xStandardMat.Diffuse.g = 1.0f;
	xStandardMat.Diffuse.b = 1.0f;
	xStandardMat.Diffuse.a = 1.0f;
	
	// Set the RGBA for ambient reflection.
	xStandardMat.Ambient.r = 1.0f;
	xStandardMat.Ambient.g = 1.0f;
	xStandardMat.Ambient.b = 1.0f;
	xStandardMat.Ambient.a = 1.0f;
	
	// Set the color and sharpness of specular highlights.
	xStandardMat.Specular.r = fSpecularBrightness;
	xStandardMat.Specular.g = fSpecularBrightness;
	xStandardMat.Specular.b = fSpecularBrightness;
	xStandardMat.Specular.a = fSpecularBrightness;
	xStandardMat.Power = fPower;
	
	// Set the RGBA for emissive color.
	xStandardMat.Emissive.r = 0.0f;
	xStandardMat.Emissive.g = 0.0f;
	xStandardMat.Emissive.b = 0.0f;
	xStandardMat.Emissive.a = 0.0f;

	EngineSetMaterial(&xStandardMat);
}


void EngineSetStandardMaterial( void )
{
ENGINEMATERIAL xStandardMat;
	
	memset( &xStandardMat, 0, sizeof( xStandardMat ) );

	// Set the RGBA for diffuse reflection.
	xStandardMat.Diffuse.r = 1.0f;
	xStandardMat.Diffuse.g = 1.0f;
	xStandardMat.Diffuse.b = 1.0f;
	xStandardMat.Diffuse.a = 1.0f;
	
	// Set the RGBA for ambient reflection.
	xStandardMat.Ambient.r = 1.0f;
	xStandardMat.Ambient.g = 1.0f;
	xStandardMat.Ambient.b = 1.0f;
	xStandardMat.Ambient.a = 1.0f;

	// Note no specular by default..

	EngineSetMaterial(&xStandardMat);
}

void	EngineDefaultState( void )
{
	if ( mpEngineDevice )
	{
		// this should set fog, lighting, ztest and everything as we'd 'normally' expect.
		EngineResetColourMode();
		EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
		EngineEnableZWrite( TRUE );
		EngineEnableZTest( TRUE );
		EngineEnableCulling( 1 );
		EngineSetVertexFormat(0);
		EngineSetStandardMaterial();
		EngineEnableLighting( FALSE );
		EngineEnableFog( 0 );
		EngineEnableBlend( 1 );
		EngineEnableTextureAddressClamp( 1 );
		mpEngineDevice->SetRenderState( D3DRS_ALPHAREF, 0x1 );
	}
}

void	EngineInitDX( LPGRAPHICSDEVICE pDevice )
{
	mpEngineDevice = pDevice;
	EngineDefaultState();

	mpEngineDevice->SetRenderState( D3DRS_ALPHAREF, 0x1 );
	mpEngineDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	mpEngineDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	mpEngineDevice->SetRenderState(	D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
	mpEngineDevice->SetRenderState(	D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2 );
	mpEngineDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
}



void	EngineInitFromInterface( void )
{
	mpEngineDevice = InterfaceGetD3DDevice();
	EngineInit();
}

void	EngineFinaliseRender( void )
{
	ShadowVolumeRender();
}

void		EngineDeactivateLight( int lightNum )
{
	mpEngineDevice->LightEnable(lightNum, FALSE);
}

ENGINE_LIGHT		mxMainLight;

#define			SHADER_MAX_NUM_POINT_LIGHTS		8
#define			SHADER_MAX_NUM_SPOT_LIGHTS		8

VECT				maxPointLightPositions[SHADER_MAX_NUM_POINT_LIGHTS];
VECT				maxPointLightColours[SHADER_MAX_NUM_POINT_LIGHTS];
int					mnNumActivePointLights = 0;
int					manPointLightSlotNums[SHADER_MAX_NUM_POINT_LIGHTS] = { 0 };
float				mafPointLightRangeSquared[SHADER_MAX_NUM_POINT_LIGHTS];

VECT				maxSpotLightPositions[SHADER_MAX_NUM_SPOT_LIGHTS];
VECT				maxSpotLightDirections[SHADER_MAX_NUM_SPOT_LIGHTS];
VECT				maxSpotLightColours[SHADER_MAX_NUM_SPOT_LIGHTS];
int					mnNumActiveSpotLights = 0;
int					manSpotLightSlotNums[SHADER_MAX_NUM_SPOT_LIGHTS] = { 0 };
//float				mafPointLightRangeSquared[SHADER_MAX_NUM_POINT_LIGHTS];


void		PixelShaderSetSpotLights( LPD3DXCONSTANTTABLE pShaderPSConstantTable )
{
D3DXHANDLE handle; 

	handle = pShaderPSConstantTable->GetConstantByName(NULL, "gNumSpotLights");
	if ( handle )
	{
		pShaderPSConstantTable->SetInt( mpEngineDevice, handle, mnNumActiveSpotLights);
	}

	handle = pShaderPSConstantTable->GetConstantByName(NULL, "gSpotLightPositions");
	if ( handle )
	{
		pShaderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)maxSpotLightPositions, 3 * mnNumActiveSpotLights);
	}

	handle = pShaderPSConstantTable->GetConstantByName(NULL, "gSpotLightDirections");
	if ( handle )
	{
		pShaderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)maxSpotLightDirections, 3 * mnNumActiveSpotLights);
	}
	
}

uint32		mulEngineFogCol = 0;
float		mfEngineFogStart = 0.0f;
float		mfEngineFogEnd = 0.0f;

void	EngineSetFogColour( uint32 uARGB )
{
	mulEngineFogCol = uARGB;
	mpEngineDevice->SetRenderState( D3DRS_FOGCOLOR, uARGB );
}


void		EngineSetFog( uint32 ulFogCol, float fFogStart, float fFogEnd )
{
	EngineSetFogColour( ulFogCol );

	mfEngineFogStart = fFogStart;
	mfEngineFogEnd = fFogEnd;
}


void		PixelShaderSetFog( LPD3DXCONSTANTTABLE pShaderPSConstantTable )
{
D3DXHANDLE handle; 

	handle = pShaderPSConstantTable->GetConstantByName(NULL, "xFogCol");
	if ( handle )
	{
	float	afFogColourRGBA[4];
	uint32	ulFogCol = 	mulEngineFogCol;

		afFogColourRGBA[0] = (float)( (ulFogCol >> 16) & 0xFF ) / 255.0f;
		afFogColourRGBA[1] = (float)( (ulFogCol >> 8) & 0xFF ) / 255.0f;
		afFogColourRGBA[2] = (float)( ulFogCol & 0xFF ) / 255.0f;
		afFogColourRGBA[3] = 1.0f;//( (ulFogCol >> 24) & 0xFF );

		pShaderPSConstantTable->SetFloatArray( mpEngineDevice, handle, afFogColourRGBA, 4 );
	}

	handle = pShaderPSConstantTable->GetConstantByName(NULL, "gFogStart");
	if ( handle )
	{
	float	fFogStart = mfEngineFogStart;
		pShaderPSConstantTable->SetFloat( mpEngineDevice, handle, fFogStart);
	}

	handle = pShaderPSConstantTable->GetConstantByName(NULL, "gFogEnd");
	if ( handle )
	{
	float	fFogEnd = mfEngineFogEnd;
		pShaderPSConstantTable->SetFloat( mpEngineDevice, handle, fFogEnd );
	}


}

void		PixelShaderSetPointLights( LPD3DXCONSTANTTABLE pShaderPSConstantTable )
{
D3DXHANDLE handle; 

	PixelShaderSetSpotLights( pShaderPSConstantTable );

	handle = pShaderPSConstantTable->GetConstantByName(NULL, "gNumPointLights");
	if ( handle )
	{
		pShaderPSConstantTable->SetInt( mpEngineDevice, handle, mnNumActivePointLights);
	}

	handle = pShaderPSConstantTable->GetConstantByName(NULL, "gPointLightPositions");
	if ( handle )
	{
		pShaderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)maxPointLightPositions, 3 * mnNumActivePointLights);
	}

	handle = pShaderPSConstantTable->GetConstantByName(NULL, "gPointLightColours");
	if ( handle )
	{
		pShaderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)maxPointLightColours, 3 * mnNumActivePointLights);
	}
	handle = pShaderPSConstantTable->GetConstantByName(NULL, "gPointLightRanges");
	if ( handle )
	{
		pShaderPSConstantTable->SetFloatArray( mpEngineDevice, handle, (float*)mafPointLightRangeSquared, mnNumActivePointLights);
	}

}

void		EngineLightingDXNewFrame( void )
{
int		nLoop;

	for( nLoop = 0; nLoop < SHADER_MAX_NUM_POINT_LIGHTS; nLoop++ )
	{
		manPointLightSlotNums[nLoop] = NOTFOUND;
	}
	mnNumActivePointLights = 0;

	for( nLoop = 0; nLoop < SHADER_MAX_NUM_SPOT_LIGHTS; nLoop++ )
	{
		manSpotLightSlotNums[nLoop] = NOTFOUND;
	}
	mnNumActiveSpotLights = 0;
}


void	EngineGetPrimaryLight( ENGINE_LIGHT* pxOut )
{
	*pxOut = mxMainLight;
}

void		EngineActivateLight( int nLightNum, ENGINE_LIGHT* pLight )
{
	if ( nLightNum == 0 )
	{
		mxMainLight = *pLight;
	}

	if ( pLight )
	{
		// Store light info to be passed to shaders..
		switch ( pLight->Type )
		{
		case POINT_LIGHT:
			{
			int		nLoop;
			int		nSlot;
			float	fRange;

				// Check if this is replacing an existing point light or adding a new one
				for( nLoop = 0; nLoop < SHADER_MAX_NUM_POINT_LIGHTS; nLoop++ )
				{
					if ( manPointLightSlotNums[nLoop] == nLightNum )
					{
						break;
					}
				}

				if ( nLoop < SHADER_MAX_NUM_POINT_LIGHTS )
				{
					nSlot = nLoop;
				}
				else
				{
					nSlot = mnNumActivePointLights;
					if ( mnNumActivePointLights < (SHADER_MAX_NUM_POINT_LIGHTS - 1) )
					{
						mnNumActivePointLights++;
					}
				}
				maxPointLightPositions[nSlot] = pLight->Position;
				fRange = pLight->Range * 0.2f;
				mafPointLightRangeSquared[nSlot] = fRange * fRange;
				maxPointLightColours[nSlot].x = pLight->Diffuse.r;
				maxPointLightColours[nSlot].y = pLight->Diffuse.g;
				maxPointLightColours[nSlot].z = pLight->Diffuse.b;
			}
			break;
		case SPOT_LIGHT:
			{
			int		nLoop;
			int		nSlot;
//			float	fRange;

				// Check if this is replacing an existing point light or adding a new one
				for( nLoop = 0; nLoop < SHADER_MAX_NUM_SPOT_LIGHTS; nLoop++ )
				{
					if ( manSpotLightSlotNums[nLoop] == nLightNum )
					{
						break;
					}
				}

				if ( nLoop < SHADER_MAX_NUM_SPOT_LIGHTS )
				{
					nSlot = nLoop;
				}
				else
				{
					nSlot = mnNumActiveSpotLights;
					if ( mnNumActiveSpotLights < (SHADER_MAX_NUM_SPOT_LIGHTS - 1) )
					{
						mnNumActiveSpotLights++;
					}
				}
				maxSpotLightPositions[nSlot] = pLight->Position;
//				fRange = pLight->Range * 0.2f;
//				mafPointLightRangeSquared[nSlot] = fRange * fRange;
				maxSpotLightColours[nSlot].x = pLight->Diffuse.r;
				maxSpotLightColours[nSlot].y = pLight->Diffuse.g;
				maxSpotLightColours[nSlot].z = pLight->Diffuse.b;
				maxSpotLightDirections[nSlot] = pLight->Direction;
			}
			break;
		}

		mpEngineDevice->SetLight(nLightNum, (GRAPHICSLIGHT*)pLight );
		mpEngineDevice->LightEnable(nLightNum, TRUE);
	}
	else
	{
		mpEngineDevice->LightEnable(nLightNum, FALSE);
	}
}





void*	EngineGetTextureDirect( int nTex )
{
IDirect3DBaseTexture9*		pxTexture = NULL;

	mpEngineDevice->GetTexture( nTex, &pxTexture );
	return( pxTexture );
}


void		EngineClearZBuffer( void )
{
	mpEngineDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER,0, 1.0f, 0 );
}
