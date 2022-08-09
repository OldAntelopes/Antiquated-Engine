
#include <stdio.h>

#include <windows.h>			// For OpenGL
#include <gl/gl.h>

#include <StandardDef.h>
#include <Interface.h>

#include "../Common/InterfaceUtil.h"
#include "VertexBufferGL.h"


void		VertexBuffer::Render( int nNumTris )
{
int		nNumVertices = nNumTris * 3;
int		loop;
float	fR, fG, fB, fA;

	glEnable (GL_TEXTURE_2D); 
	glBegin (GL_TRIANGLES);

	for ( loop = 0; loop < nNumVertices; loop++ )
	{
		InterfaceUnpackCol( pxVertexMem[loop].color, &fR, &fG, &fB, &fA );
		glColor4f( fR, fG, fB, fA );

		glTexCoord2f ( pxVertexMem[loop].tu, pxVertexMem[loop].tv );
		glVertex3f ( pxVertexMem[loop].x, pxVertexMem[loop].y, 0.0f );//pxVertexMem[loop].z );
	}

	glEnd ();
    glDisable (GL_TEXTURE_2D); /* disable texture mapping */

}


void		VertexBuffer::Unlock( void )
{

}


FLATVERTEX*		VertexBuffer::Lock( void )
{
	return( pxVertexMem );
}


VertexBuffer::VertexBuffer( int nNumVertices )
{
	pxVertexMem = (FLATVERTEX*)( malloc( nNumVertices * sizeof(FLATVERTEX) ) );
	nAddPos = 0;
	nMaxVertices = nNumVertices;
}

VertexBuffer::~VertexBuffer( void )
{

}


//----------------------------------------------------------------------------------------
int		VertexBufferCreate( int nNumVertices, VertexBuffer** ppVertexBuffer )
{
VertexBuffer*		pNewBuffer = new VertexBuffer( nNumVertices );

	*ppVertexBuffer = pNewBuffer;
	return( -1 );
}

