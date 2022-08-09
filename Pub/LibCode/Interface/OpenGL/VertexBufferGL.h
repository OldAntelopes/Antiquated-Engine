#ifndef INTERFACE_VERTEX_BUFFER_OPENGL_H
#define INTERFACE_VERTEX_BUFFER_OPENGL_H

#include "../Common/Overlays/Overlays.h"		// for FLATVERTEX.. mm, move that to more common area?

class VertexBuffer
{
public:
	VertexBuffer( int nNumVertices );
	~VertexBuffer();

	FLATVERTEX*		Lock( void );
	void			Unlock( void );

	void			Render( int nNumVerts );

private:
	FLATVERTEX*		pxVertexMem;
	int				nAddPos;
	int				nMaxVertices;
};




// -------- C-style interfaces

extern int		VertexBufferCreate( int nNumVertices, VertexBuffer** );





#endif // #ifndef INTERFACE_VERTEX_BUFFER_OPENGL_H
