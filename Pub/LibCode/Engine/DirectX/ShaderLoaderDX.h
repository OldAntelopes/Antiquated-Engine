#ifndef ENGINE_SHADER_LOADER_H
#define ENGINE_SHADER_LOADER_H

#include "EngineDX.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern LPGRAPHICSVERTEXSHADER		EngineLoadVertexShader( const char* szShaderName, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable, int flags );
extern LPGRAPHICSPIXELSHADER		EngineLoadPixelShader( const char* szShaderName, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable, int flags );

extern void							EngineReleaseConstantBuffer( LPGRAPHICSCONSTANTBUFFER* ppConstantBuffer );

extern void	EngineShadersStandardVertexDeclaration( int mode );

extern void				EngineShaderConstantsSetMatrix( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, const ENGINEMATRIX* pxMatrix );
extern void				EngineShaderConstantsSetFloat( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, float fValue );
extern void				EngineShaderConstantsSetVect( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, const VECT* pxVect );

#ifdef __cplusplus
}
#endif

#endif