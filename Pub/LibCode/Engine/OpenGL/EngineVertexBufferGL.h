#ifndef ENGINE_VERTEX_BUFFER_OPENGL_H
#define ENGINE_VERTEX_BUFFER_OPENGL_H



class EngineVertexBufferGL
{
public:
	EngineVertexBufferGL( int nNumVertices );
	~EngineVertexBufferGL();

	CUSTOMVERTEX*		Lock( void );
	void				Unlock( void );

private:
	CUSTOMVERTEX*		pxVertexMem;
	int					nAddPos;
	int					nMaxVertices;
};








#endif // #ifndef ENGINE_VERTEX_BUFFER_OPENGL_H
