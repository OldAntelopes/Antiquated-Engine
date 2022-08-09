#ifndef ENGINE_GL_H
#define ENGINE_GL_H

typedef void*	LPGRAPHICSPIXELSHADER;
typedef void*	LPGRAPHICSVERTEXSHADER;

extern void				EngineSetPixelShader( LPGRAPHICSPIXELSHADER, const char* szShaderName );
extern void				EngineSetVertexShader( LPGRAPHICSVERTEXSHADER, const char* szShaderName );



#endif