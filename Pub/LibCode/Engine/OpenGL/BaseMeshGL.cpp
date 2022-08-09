#include <windows.h>		// For OpenGL
#include <gl/gl.h>

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include "../ModelRendering.h"

#include "EngineVertexBufferGL.h"
#include "BaseMeshGL.h"


BaseMeshGL::BaseMeshGL()
{
	// TODO
	m_puwIndices = NULL;
	m_pVertexBuffer = NULL;		// Will need multiple ones of these to do multiple materials
	m_ulNumVertices = 0;
	m_ulNumFaces = 0;

}


BaseMeshGL::~BaseMeshGL()
{
	// TODO

}

void	BaseMeshGL::Release( void )
{
	// TODO
}



int	BaseMeshGL::GetNumFaces( void )
{
	return( m_ulNumFaces );
}


int	BaseMeshGL::GetNumVertices( void )
{
	return( m_ulNumVertices );
}


void	BaseMeshGL::ReorderByMaterial( fnVertexRemapCallback, void*  )
{
	// TODO

}


void	BaseMeshGL::RayTest( const VECT* pRayPos, const VECT* pRayDir, BOOL* pbHit, ulong* pFaceIndex, float* pU, float* pV, float* pfDist, void* pIntersectBuffer, ulong* pulHitCount )
{
	// TODO
}

int		BaseMeshGL::LockVertexBuffer( int flags, unsigned char** ppVertices )
{
CUSTOMVERTEX*	pxVertexBuffer;

	if ( m_pVertexBuffer )
	{
		pxVertexBuffer = m_pVertexBuffer->Lock();
	}
	*ppVertices = (byte*)( pxVertexBuffer );
	return( 0 );
}

int		BaseMeshGL::UnlockVertexBuffer( void )
{
	if ( m_pVertexBuffer )
	{
		m_pVertexBuffer->Unlock();
	}
	return( 0 );
}


int		BaseMeshGL::LockIndexBuffer( int flags, unsigned char** ppIndices )
{
	*ppIndices = (byte*)( m_puwIndices );
	return( 0 );
}

int		BaseMeshGL::UnlockIndexBuffer( void )
{
	// TODO
	return( 0 );
}


int		BaseMeshGL::LockAttributeBuffer( int flags, unsigned char** ppAttributes )
{
	// TODO
	return( 0 );
}

void	BaseMeshGL::UnlockAttributeBuffer()
{
	// TODO
}

void	BaseMeshGL::Create( int nNumFaces, int nNumVertices, int nFlags )
{
	// TODO
	m_puwIndices = (ushort*)( malloc( nNumFaces * 3 * sizeof(ushort) ) );
	memset( m_puwIndices, 0, nNumFaces * 3 * sizeof(ushort) );

	m_pVertexBuffer = new EngineVertexBufferGL( nNumVertices );

	m_ulNumVertices = (ulong)( nNumVertices );
	m_ulNumFaces = (ulong)( nNumFaces );
}

BOOL	BaseMeshGL::Is32BitIndexBuffer( void )
{
	return( FALSE );
}

void	BaseMeshGL::DrawSubset( int materialNum )
{
CUSTOMVERTEX*	pxVertexBuffer;
ulong	uLoop;
ulong	indexBase = 0;
ulong	vertex = 0;

	glEnable (GL_TEXTURE_2D); 
	glBegin (GL_TRIANGLES);

	if ( m_pVertexBuffer )
	{
		pxVertexBuffer = m_pVertexBuffer->Lock();

		for ( uLoop = 0; uLoop < m_ulNumFaces; uLoop++ )
		{
			indexBase = uLoop * 3;
			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

			vertex = m_puwIndices[indexBase];
			glTexCoord2f ( pxVertexBuffer[vertex].tu, pxVertexBuffer[vertex].tv );
			glVertex3f ( pxVertexBuffer[vertex].position.x, pxVertexBuffer[vertex].position.y, pxVertexBuffer[vertex].position.z );

			vertex = m_puwIndices[indexBase+1];
			glTexCoord2f ( pxVertexBuffer[vertex].tu, pxVertexBuffer[vertex].tv );
			glVertex3f ( pxVertexBuffer[vertex].position.x, pxVertexBuffer[vertex].position.y, pxVertexBuffer[vertex].position.z );

			vertex = m_puwIndices[indexBase+2];
			glTexCoord2f ( pxVertexBuffer[vertex].tu, pxVertexBuffer[vertex].tv );
			glVertex3f ( pxVertexBuffer[vertex].position.x, pxVertexBuffer[vertex].position.y, pxVertexBuffer[vertex].position.z );
		}
		m_pVertexBuffer->Unlock();
	}

	glEnd ();
    glDisable (GL_TEXTURE_2D); /* disable texture mapping */
}
