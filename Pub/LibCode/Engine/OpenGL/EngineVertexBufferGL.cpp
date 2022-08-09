
#include <stdio.h>

#include <windows.h>		// For OpenGL
#include <gl/gl.h>

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>

#include "../ModelRendering.h"

#include "EngineVertexBufferGL.h"


void		EngineVertexBufferGL::Unlock( void )
{

}


CUSTOMVERTEX*		EngineVertexBufferGL::Lock( void )
{
	return( pxVertexMem );
}


EngineVertexBufferGL::EngineVertexBufferGL( int nNumVertices )
{
	pxVertexMem = (CUSTOMVERTEX*)( malloc( nNumVertices * sizeof(CUSTOMVERTEX) ) );
	nAddPos = 0;
	nMaxVertices = nNumVertices;
}

EngineVertexBufferGL::~EngineVertexBufferGL( void )
{

}



//====================================
VERTEX_BUFFER_HANDLE		EngineCreateVertexBuffer( int nMaxVertices, int nFlags )
{
	// TODO
	return( NOTFOUND );
}
BOOL		EngineVertexBufferRender( VERTEX_BUFFER_HANDLE nHandle, ENGINEPRIMITIVE_TYPE nPrimType )
{
	// TODO

	return( FALSE );
}

void		EngineVertexBufferReset( VERTEX_BUFFER_HANDLE  nHandle )
{
	// TODO

}

void		EngineVertexBufferFree( VERTEX_BUFFER_HANDLE nHandle )
{
	// TODO
}

BOOL		EngineVertexBufferLock( VERTEX_BUFFER_HANDLE nHandle, BOOL bClean )
{
	// TODO
	return( FALSE );
}

ENGINEBUFFERVERTEX*		EngineVertexBufferGetBufferPointer( VERTEX_BUFFER_HANDLE nHandle, int nNumVertsRequired )
{
	// TODO
	return( NULL );

}

void		EngineVertexBufferUnlockColourStream( VERTEX_BUFFER_HANDLE handle )
{
	// TODO

}

unsigned long*	EngineVertexBufferLockColourStream( VERTEX_BUFFER_HANDLE handle , int* pnStride )
{
	// TODO
	return( NULL );
}

VECT*	EngineVertexBufferLockPositionStream( VERTEX_BUFFER_HANDLE handle, int* pnStride )
{
	// TODO
	return( NULL );
}
float*	EngineVertexBufferLockUVStream( VERTEX_BUFFER_HANDLE handle, int* pnStride )
{
	// TODO
	return( NULL );
}

VECT*	EngineVertexBufferLockNormalStream( VERTEX_BUFFER_HANDLE handle, int* pnStride )
{
	// TODO
	return( NULL );
}


void		EngineVertexBufferAddVertsUsed( VERTEX_BUFFER_HANDLE, int nNumVertsUsed )
{


}

BOOL		EngineVertexBufferUnlock( VERTEX_BUFFER_HANDLE )
{

	return( FALSE );
}

INDEX_BUFFER_HANDLE		EngineCreateIndexBuffer( int nMaxIndices, int nType )
{
	// TODO
	return( NOTFOUND );
}

unsigned short*			EngineIndexBufferLock( INDEX_BUFFER_HANDLE, int flags )
{
	// TODO
	return( NULL );

}

void						EngineIndexBufferUnlock( INDEX_BUFFER_HANDLE )
{
	// TODO
}

BOOL						EngineIndexBufferRender( INDEX_BUFFER_HANDLE, VERTEX_BUFFER_HANDLE, int numPolys, int flags )
{
	// TODO
	return( FALSE );
}

void						EngineIndexBufferFree( INDEX_BUFFER_HANDLE )
{
	// TODO
}
