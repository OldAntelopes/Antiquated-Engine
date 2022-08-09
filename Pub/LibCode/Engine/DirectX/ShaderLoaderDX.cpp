
#include <stdio.h>
#include "EngineDX.h"

#include <StandardDef.h>
#include <Engine.h>
#include <Interface.h>

#include "ShaderLoaderDX.h"

#ifdef TUD9
LPDIRECT3DVERTEXDECLARATION9	mpStandardVertexDeclaration = NULL; //VertexDeclaration (NEW)
LPDIRECT3DVERTEXDECLARATION9	mpLandscapeVertexDeclaration = NULL; //VertexDeclaration (NEW)
#endif

void	EngineShadersStandardVertexDeclaration( int mode )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineShadersStandardVertexDeclaration TBI" );
#else
	switch( mode )
	{
	case 0:
	default:
		mpEngineDevice->SetVertexDeclaration(mpStandardVertexDeclaration);
		break;
	case 1:
		mpEngineDevice->SetVertexDeclaration(mpLandscapeVertexDeclaration);
		break;
	}
#endif
}

LPGRAPHICSVERTEXSHADER		EngineLoadVertexShader( const char* szShaderName, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable, int flags )
{
LPGRAPHICSVERTEXSHADER			pVertexShader = NULL;

#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineLoadVertexShader TBI" );
#else
HRESULT		ret;
LPD3DXBUFFER		pErrorBuffer = NULL;
LPD3DXBUFFER		pShaderCode = NULL; //Temporary buffer (NEW)

char		acShaderFilename[256];

	sprintf( acShaderFilename, "Data\\Shaders\\%s.vsh", szShaderName );

	if ( mpStandardVertexDeclaration == NULL )
	{
	// Standard ENGINEVERTEX with two UVS
	D3DVERTEXELEMENT9 declStandard[] = { { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
							 { 0,12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
							 { 0,24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
						     { 0,28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
						     { 0,36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
								 D3DDECL_END()  };
	D3DVERTEXELEMENT9 declLandscape[] = { { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
							 { 0,12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
							 { 0,24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
						     { 0,28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
						     { 0,36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
							 { 0,44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
								 D3DDECL_END()  };
		mpEngineDevice->CreateVertexDeclaration( declStandard, &mpStandardVertexDeclaration );
		mpEngineDevice->CreateVertexDeclaration( declLandscape, &mpLandscapeVertexDeclaration );
	}

	ret = D3DXCompileShaderFromFile(acShaderFilename,    //filepath
											 NULL,            //macro's
											 NULL,            //includes
											"vs_main",       //main function
											"vs_3_0",        //shader profile
											 0,               //flags
											 &pShaderCode,           //compiled operations
											&pErrorBuffer,            //errors
											ppxConstantTable ); //constants
	if(FAILED(ret))
	{
		if ( pErrorBuffer )
		{
		char*		pcErrors = (char*)pErrorBuffer->GetBufferPointer();
#ifdef TOOL
			printf( pcErrors );
#else
#ifdef _DEBUG
			MessageBox(NULL, pcErrors, "Error", MB_OK);
#endif
#endif
		}
		else
		{
		FILE*		pFile = fopen( acShaderFilename, "rb" );

			if ( pFile == NULL )
			{
				printf( "File not found\n" );
			}
			else
			{
				fclose( pFile );
			}
		}
//		PrintConsoleCR( "*Warning*: Vertex shader load failed - Some graphical effects may be missing", 0 );
		return( NULL );
	}

	mpEngineDevice->CreateVertexShader((DWORD*)pShaderCode->GetBufferPointer(), &pVertexShader );
	pShaderCode->Release();
#endif

	return( pVertexShader );

}




LPGRAPHICSPIXELSHADER		EngineLoadPixelShader( const char* szShaderName, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable, int flags )
{
HRESULT		ret;
LPGRAPHICSPIXELSHADER			pPixelShader = NULL;

#ifdef TUD11
	PANIC_IF( TRUE, "EngineLoadPixelShader DX11 TBI" );
	
#else
LPD3DXBUFFER		pErrorBuffer = NULL;
LPD3DXBUFFER		pShaderCode = NULL; //Temporary buffer (NEW)
char		acShaderFilename[256];
FILE*		pFile;

	sprintf( acShaderFilename, "Data\\Shaders\\%s.psh", szShaderName );

	pFile = fopen( acShaderFilename, "rb" );
	if ( pFile )
	{
		fclose( pFile );
	}
	else
	{
		PANIC_IF( TRUE, "Shader file not found" );
	}
	//------------ Init Pixel Shader
	ret = D3DXCompileShaderFromFile(acShaderFilename,  //filepath
									   NULL,          //macro's            
									   NULL,          //includes           
									   "ps_main",     //main function      
									   "ps_3_0",      //shader profile     
									   0,             //flags              
									   &pShaderCode,         //compiled operations
									   &pErrorBuffer,          //errors
									   ppxConstantTable );         //constants
	if(FAILED(ret))
	{
		if ( pErrorBuffer )
		{
		char*		pcErrors = (char*)pErrorBuffer->GetBufferPointer();
			printf( pcErrors );
#ifdef TOOL
			MessageBox(NULL, pcErrors, "Error", MB_OK);
#else
#ifdef _DEBUG
			MessageBox(NULL, pcErrors, "Error", MB_OK);
#endif
#endif
		}
//		PrintConsoleCR( "*Warning*: Pixel shader load failed - Some graphical effects may be missing", 0 );
		return( NULL );
	}
		
	mpEngineDevice->CreatePixelShader((DWORD*)pShaderCode->GetBufferPointer(), &pPixelShader);
	pShaderCode->Release();
#endif

	return( pPixelShader );
}



void		EngineReleaseConstantBuffer( LPGRAPHICSCONSTANTBUFFER* ppConstantBuffer )
{
#ifdef TUD11
	SAFE_FREE( *ppConstantBuffer );
#else
	SAFE_RELEASE( *ppConstantBuffer );
#endif

}

void				EngineShaderConstantsSetFloat( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, float fValue )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineShaderConstantsSetFloat TBI" );
#else
	D3DXHANDLE handle; 

	handle = pConstantBuffer->GetConstantByName(NULL, szConstantName);
	if ( handle )
	{
		pConstantBuffer->SetFloat( mpEngineDevice, handle, fValue );
	}
#endif

}

void				EngineShaderConstantsSetVect( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, const VECT* pxVect )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineShaderConstantsSetVect TBI" );
#else
	D3DXHANDLE handle; 

	handle = pConstantBuffer->GetConstantByName(NULL, szConstantName);
	if ( handle )
	{
		pConstantBuffer->SetFloatArray( mpEngineDevice, handle, (const float*)pxVect, 3 );
	}
#endif
}

void				EngineShaderConstantsSetMatrix( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, const ENGINEMATRIX* pxMatrix )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineShaderConstantsSetMatrix TBI" );
#else
	D3DXHANDLE handle; 

	handle = pConstantBuffer->GetConstantByName(NULL, szConstantName);
	if ( handle )
	{
		pConstantBuffer->SetMatrix( mpEngineDevice, handle, (D3DXMATRIX*)pxMatrix );
	}
#endif

}
