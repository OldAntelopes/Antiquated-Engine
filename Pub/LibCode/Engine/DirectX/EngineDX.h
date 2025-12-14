#ifndef ENGINE_DX_H
#define ENGINE_DX_H

#ifdef USING_OPENGL
//#include "OpenGL/BaseMeshGL.h"
#else
#ifdef TUD11
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#else
#include "../../../Include/DirectX/d3dx9.h"
#include "../../../Include/DirectX/dxerr9.h"
#endif
#endif

#include "Engine.h"

#define USE_D3DEX_INTERFACE

#ifdef TUD11
typedef ID3D11Buffer			IGRAPHICSVERTEXBUFFER;
typedef ID3D11Buffer			IGRAPHICSINDEXBUFFER;
typedef ID3D11Texture2D*		LPGRAPHICSTEXTURE;
typedef ID3D11Device*	 		LPGRAPHICSDEVICE;
typedef ID3D11DeviceContext*	LPGRAPHICSDEVICECONTEXT;
typedef ID3D11Texture2D			IGRAPHICSSURFACE;
typedef void**					VERTEX_LOCKTYPE;
typedef DirectX::XMFLOAT3		D3DXVECTOR3;
typedef DirectX::XMFLOAT4		D3DXVECTOR4;
typedef DirectX::XMFLOAT4		D3DCOLOR;
typedef ENGINEMATRIX			D3DXMATRIX;
typedef DirectX::XMFLOAT4		D3DXQUATERNION;
typedef ID3D11PixelShader*		LPGRAPHICSPIXELSHADER;
typedef ID3D11VertexShader*		LPGRAPHICSVERTEXSHADER;
typedef void*					LPGRAPHICSCONSTANTBUFFER;
typedef DXGI_FORMAT				IGRAPHICSFORMAT;

//extern LPDIRECT3DDEVICE9        mpInterfaceD3DDevice; // Our rendering device
//typedef D3DCAPS9				GRAPHICSCAPS;
//typedef D3DADAPTER_IDENTIFIER9	GRAPHICSADAPTER_IDENTIFIER;
//typedef D3DLIGHT9				GRAPHICSLIGHT;
//typedef D3DMATERIAL9			GRAPHICSMATERIAL;
#else
#ifdef USE_D3DEX_INTERFACE
extern LPDIRECT3DDEVICE9EX      mpInterfaceD3DDevice; // Our rendering device
typedef LPDIRECT3DDEVICE9EX		LPGRAPHICSDEVICE;
#else
extern LPDIRECT3DDEVICE9        mpInterfaceD3DDevice; // Our rendering device
typedef LPDIRECT3DDEVICE9		LPGRAPHICSDEVICE;
#endif
typedef IDirect3DVertexBuffer9	IGRAPHICSVERTEXBUFFER;
typedef IDirect3DIndexBuffer9   IGRAPHICSINDEXBUFFER;
typedef LPDIRECT3DTEXTURE9		LPGRAPHICSTEXTURE;
typedef D3DMATERIAL9			GRAPHICSMATERIAL;
typedef IDirect3DSurface9*		LPGRAPHICSSURFACE;
typedef D3DCAPS9				GRAPHICSCAPS;
typedef void**					VERTEX_LOCKTYPE;
typedef D3DADAPTER_IDENTIFIER9	GRAPHICSADAPTER_IDENTIFIER;
typedef D3DLIGHT9				GRAPHICSLIGHT;
typedef LPDIRECT3DPIXELSHADER9	LPGRAPHICSPIXELSHADER;
typedef LPDIRECT3DVERTEXSHADER9	LPGRAPHICSVERTEXSHADER;
typedef LPD3DXCONSTANTTABLE		LPGRAPHICSCONSTANTBUFFER;
typedef D3DFORMAT				IGRAPHICSFORMAT;
#endif

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	
// EngineDX.h
// For internal DX-specific/engine stuff only..
// Included only by files inside the Engine.lib
extern LPGRAPHICSDEVICE			mpEngineDevice;
#ifdef TUD11
extern LPGRAPHICSDEVICECONTEXT	mpEngineDeviceContext;
#endif

IGRAPHICSFORMAT		EngineDXGetGraphicsFormat( eSurfaceFormat format );
eSurfaceFormat		EngineDXGetSurfaceFormat( IGRAPHICSFORMAT format );


void				EngineSetPixelShader( LPGRAPHICSPIXELSHADER, const char* szShaderName );
void				EngineSetVertexShader( LPGRAPHICSVERTEXSHADER, const char* szShaderName );

extern void		EngineLightingDXNewFrame( void );
extern void		PixelShaderSetPointLights( LPD3DXCONSTANTTABLE pShaderPSConstantTable );
extern void		PixelShaderSetFog( LPD3DXCONSTANTTABLE pShaderPSConstantTable );

extern int		EngineGetNumVertexBuffersAllocated( void );
extern void		EngineVertexBufferTrackingListAllocated( char* );
extern int		EngineTextureManagerGetNumRenderTargets( void );
extern void		EngineRenderTargetsTrackingListAllocated( char* );

extern void		EngineMathsInitSinTable( void );
extern void		EngineMathsFreeSinTable( void );

extern BOOL	 msbEngineMaterialBlendOverride;

#ifdef __cplusplus
}
#endif

#endif //#ifndef ENGINE_DX_H