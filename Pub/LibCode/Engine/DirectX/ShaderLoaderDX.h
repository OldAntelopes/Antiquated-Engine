#ifndef ENGINE_SHADER_LOADER_H
#define ENGINE_SHADER_LOADER_H

#include "EngineDX.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern LPGRAPHICSVERTEXSHADER		EngineLoadVertexShader( const char* szShaderName, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable, int flags );
extern LPGRAPHICSPIXELSHADER		EngineLoadPixelShader( const char* szShaderName, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable, int flags );
// The same compiled with one preprocessor define set to 1 (NULL = none)
extern LPGRAPHICSPIXELSHADER		EngineLoadPixelShaderWithDefine( const char* szShaderName, const char* szDefine, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable );

// As D3DXCompileShaderFromFile (no include handler), through the compiled shader cache when EngineSetShaderCache (Engine.h) has
// turned it on; exactly D3DXCompileShaderFromFile when it hasn't. Safe on a worker thread. pbFromCache (optional) says which happened.
extern HRESULT						EngineCompileShaderFileCached( const char* szFilename, const D3DXMACRO* pxMacros, const char* szEntry, const char* szProfile, DWORD ulFlags,
																   LPD3DXBUFFER* ppCode, LPD3DXBUFFER* ppErrors, LPD3DXCONSTANTTABLE* ppTable, BOOL* pbFromCache );

extern void							EngineReleaseConstantBuffer( LPGRAPHICSCONSTANTBUFFER* ppConstantBuffer );

extern void	EngineShadersStandardVertexDeclaration( int mode );

extern void				EngineShaderConstantsSetMatrix( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, const ENGINEMATRIX* pxMatrix );
extern void				EngineShaderConstantsSetFloat( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, float fValue );
extern void				EngineShaderConstantsSetVect( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, const VECT* pxVect );

#ifdef __cplusplus
}
#endif

#endif