#ifndef INTERFACE_TYPES_DX_H
#define INTERFACE_TYPES_DX_H

#define USE_D3DEX_INTERFACE

#ifdef TUD11
#include <d3d11.h>
#else
#include "../../../Include/DirectX/d3dx9.h"
#endif

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	
#ifndef MIT_TYPES
#define MIT_TYPES
typedef unsigned short			ushort;
typedef unsigned char			uchar;
typedef unsigned int			uint;
#endif

#ifndef UINT32_DEFINED
typedef unsigned __int32		uint32;
#define UINT32_DEFINED
#endif

#if (DIRECT3D_VERSION>=0x0900)
#ifdef USE_D3DEX_INTERFACE
typedef LPDIRECT3DDEVICE9EX		LPGRAPHICSDEVICE;
typedef LPDIRECT3D9EX			LPGRAPHICS;
#else
typedef LPDIRECT3DDEVICE9		LPGRAPHICSDEVICE;
typedef LPDIRECT3D9				LPGRAPHICS;
#endif
typedef LPDIRECT3DTEXTURE9		LPGRAPHICSTEXTURE;
typedef IDirect3DSurface9		IGRAPHICSSURFACE;

#else		// --- less than DX9 ! oldscool
typedef LPDIRECT3DDEVICE8		LPGRAPHICSDEVICE;
typedef LPDIRECT3D8				LPGRAPHICS;
typedef LPDIRECT3DTEXTURE8		LPGRAPHICSTEXTURE;
#endif

#ifdef TUD11
typedef ID3D11Texture2D*		LPGRAPHICSTEXTURE;
typedef ID3D11Texture2D*		LPGRAPHICSSURFACE;
typedef ID3D11Buffer			IGRAPHICSVERTEXBUFFER;
typedef ID3D11Device*			LPGRAPHICSDEVICE;
#else
typedef IDirect3DVertexBuffer9	IGRAPHICSVERTEXBUFFER;
typedef LPDIRECT3DTEXTURE9		LPGRAPHICSTEXTURE;
typedef D3DMATERIAL9			GRAPHICSMATERIAL;
typedef IDirect3DSurface9		IGRAPHICSSURFACE;
typedef D3DCAPS9				GRAPHICSCAPS;
typedef void**					VERTEX_LOCKTYPE;
typedef D3DADAPTER_IDENTIFIER9	GRAPHICSADAPTER_IDENTIFIER;
#endif


#ifdef __cplusplus
}
#endif

#endif
