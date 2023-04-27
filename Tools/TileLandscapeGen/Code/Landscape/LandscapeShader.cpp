
#ifndef SERVER

#include <d3dx9shader.h>
#include <d3dx9.h>

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#include "../LibCode/Engine/DirectX/ShaderLoaderDX.h"

#include "LandscapeHeightmap.h"
#include "SourceTextures.h"
#include "LandscapeCoords.h"
#include "LandscapeShader.h"

#define D3DFVF_LANDRENDERVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX3|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1)|D3DFVF_TEXCOORDSIZE3(2))



//---------------------------------------------------------------------------------------------------------------------------------------

BOOL	mbLandscapeShaderHasInitialised = FALSE;
BOOL	mbLandscapeShaderHasShaders = FALSE;

LPDIRECT3DPIXELSHADER9			mpLandscapeShaderPixelShader = NULL; 
LPDIRECT3DVERTEXSHADER9			mpLandscapeShaderVertexShader = NULL; 
LPD3DXCONSTANTTABLE				mpLandscapeShaderConstantTable = NULL; 
LPD3DXCONSTANTTABLE				mpLandscapeShaderPSConstantTable = NULL; 

#define		NUM_GEN_TEXTURE_SOURCES		8

int		mahLandscapeShaderSourceTextureHandles[NUM_GEN_TEXTURE_SOURCES] = { 0, 0, 0, 0,0,0,0,0 };
int		mhGrassNormal = NOTFOUND;

int		manDefaultHeightValues[8] = { 255, 200, 176, 150, 112, 70, 60, 0 };

//---------------------------------------------------------------------------------------------------------------------------------------

void LandscapeShaderSetSourceTexture( int nInputNum, TEXTURE_HANDLE hTexture  )
{
	mahLandscapeShaderSourceTextureHandles[nInputNum] = hTexture;
}



/***************************************************************************
 * Function    : LandscapeShaderInitShaders
 * 
 * Description : Load landscape shaders and any related static resources
 ***************************************************************************/
void		LandscapeShaderInitShaders( void )
{
	if ( mbLandscapeShaderHasInitialised == FALSE )
	{
		mbLandscapeShaderHasInitialised = TRUE;

		mpLandscapeShaderVertexShader = EngineLoadVertexShader( "Landscape", &mpLandscapeShaderConstantTable, 0 );
		mpLandscapeShaderPixelShader = EngineLoadPixelShader( "Landscape", &mpLandscapeShaderPSConstantTable, 0 );

		if ( ( mpLandscapeShaderVertexShader ) &&
			 ( mpLandscapeShaderPixelShader ) )
		{
			mbLandscapeShaderHasShaders = TRUE;
		}
		else
		{
			SysDebugPrint( "Warning: Shader load failed - some graphical effects may be missing. Try microsoft.com/DirectX for directX update?", COL_WARNING );
		}	
	}
}

void		LandscapeShaderUpdateFrame( void )
{


}

void		LandscapeShaderCleanup( void )
{
LPDIRECT3DDEVICE9		pd3dDevice = EngineGetDXDevice();

	pd3dDevice->SetTexture( 1, NULL );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTexture( 2, NULL );
	pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTexture( 3, NULL );
	pd3dDevice->SetTextureStageState( 3, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTexture( 4, NULL );
	pd3dDevice->SetTextureStageState( 4, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	EngineShadersStandardVertexDeclaration(0);
	pd3dDevice->SetVertexShader( NULL );
	pd3dDevice->SetPixelShader( NULL );
//	pd3dDevice->SetVertexShader( NULL );

}

int*		LandscapeShaderGetRangeConstants( void )
{
	return( manDefaultHeightValues );
}

void		LandscapeShaderSetRangeConstants( int* anHeightValues )
{
int		nLoop;

	for ( nLoop = 0; nLoop < 8; nLoop++ )
	{
		manDefaultHeightValues[nLoop] = anHeightValues[nLoop];
	}
}

void		LandscapeShaderGetRangeConstants( float* pfHeightValues )
{
int		anHeightValues[8];
int		nLoop;
float	fHeightValuesToWorldScale = 1.0f;

	for ( nLoop = 0; nLoop < 8; nLoop++ )
	{
		anHeightValues[nLoop] = manDefaultHeightValues[nLoop];
		pfHeightValues[nLoop] = anHeightValues[nLoop] * fHeightValuesToWorldScale;
	}
}

VECT		mxLandscapeShaderAmbientCol = { 0.5f, 0.5f, 0.5f };
VECT		mxLandscapeShaderDiffuseCol = { 0.5f, 0.5f, 0.5f };

void		LandscapeShaderSetAmbientColour( float fR, float fG, float fB )
{
	mxLandscapeShaderAmbientCol.x = fR;
	mxLandscapeShaderAmbientCol.y = fG;
	mxLandscapeShaderAmbientCol.z = fB;
}

void		LandscapeShaderSetDiffuseColour( float fR, float fG, float fB )
{
	mxLandscapeShaderDiffuseCol.x = fR;
	mxLandscapeShaderDiffuseCol.y = fG;
	mxLandscapeShaderDiffuseCol.z = fB;
}


/***************************************************************************
 * Function    : LandscapeShaderBeginRender
 * 
 * Description : Activate the landscape shader, setting up any required textures and render states etc
 ***************************************************************************/
BOOL		LandscapeShaderBeginRender( void )
{
	if ( ( mbLandscapeShaderHasShaders == TRUE ) &&
		 ( mpLandscapeShaderConstantTable != NULL ) )
	{
	D3DXMATRIX matWorld, matView, matProj;
	int			nLoop;
	LPDIRECT3DDEVICE9		pd3dDevice = EngineGetDXDevice();

//		if ( IsRenderingRef
		EngineActivateClippingPlane( FALSE, 0.0f );
		pd3dDevice->GetTransform(D3DTS_WORLD, &matWorld);
		pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
		pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);
 
		D3DXMATRIX matWorldViewProj = matWorld * matView * matProj;
		D3DXHANDLE handle; 

		handle = mpLandscapeShaderConstantTable->GetConstantByName(NULL, "matWorld");
		if ( handle )
		{
			mpLandscapeShaderConstantTable->SetMatrix(pd3dDevice, handle, &matWorld);		
		}

		handle = mpLandscapeShaderConstantTable->GetConstantByName(NULL, "matView");
		if ( handle )
		{
			mpLandscapeShaderConstantTable->SetMatrix(pd3dDevice, handle, &matView);		
		}

		handle = mpLandscapeShaderConstantTable->GetConstantByName(NULL, "matProj");
		if ( handle )
		{
			mpLandscapeShaderConstantTable->SetMatrix(pd3dDevice, handle, &matProj);		
		}

		handle = mpLandscapeShaderConstantTable->GetConstantByName(NULL, "EyePosition");
		if ( handle )
		{
		VECT	xEyePosition = *EngineCameraGetPos();
			mpLandscapeShaderConstantTable->SetFloatArray( pd3dDevice, handle, (float*)&xEyePosition, 3 );
		}

		if ( mpLandscapeShaderPSConstantTable )
		{
		float	afHeightValues[8];
		float	afLayerHeights[4];
		float		fSeaHeight = 0.0f;

			LandscapeShaderGetRangeConstants( afHeightValues );

			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "gClipHeight");
			if ( handle )
			{
				mpLandscapeShaderPSConstantTable->SetFloat( pd3dDevice, handle, fSeaHeight);
			}

			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "gAmbientColor");
			if ( handle )
			{
			float	afCol[4] = { mxLandscapeShaderAmbientCol.x, mxLandscapeShaderAmbientCol.y, mxLandscapeShaderAmbientCol.z, 1.0f };
				mpLandscapeShaderPSConstantTable->SetFloatArray( pd3dDevice, handle, (float*)( afCol ), 4 );
			}

			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "gDiffuseColor");
			if ( handle )
			{
			float	afCol[4] = { mxLandscapeShaderDiffuseCol.x, mxLandscapeShaderDiffuseCol.y, mxLandscapeShaderDiffuseCol.z, 1.0f };
				mpLandscapeShaderPSConstantTable->SetFloatArray( pd3dDevice, handle, (float*)( afCol ), 4 );
			}	
			
			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "xFogCol");
			if ( handle )
			{
			float	afFogColourRGBA[4];
			uint32	ulFogCol = 	0;//LandscapeRenderGetFogColour();

				afFogColourRGBA[0] = (float)( (ulFogCol >> 16) & 0xFF ) / 255.0f;
				afFogColourRGBA[1] = (float)( (ulFogCol >> 8) & 0xFF ) / 255.0f;
				afFogColourRGBA[2] = (float)( ulFogCol & 0xFF ) / 255.0f;
				afFogColourRGBA[3] = 1.0f;//( (ulFogCol >> 24) & 0xFF );

				mpLandscapeShaderPSConstantTable->SetFloatArray( pd3dDevice, handle, afFogColourRGBA, 4 );
			}

			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "gFogStart");
			if ( handle )
			{
			float	fFogStart = 0.0f;//LandscapeRenderGetFogStart();
				mpLandscapeShaderPSConstantTable->SetFloat( pd3dDevice, handle, fFogStart );
			}

			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "gFogEnd");
			if ( handle )
			{
			float	fFogEnd = 0.0f;//LandscapeRenderGetFogEnd();
				mpLandscapeShaderPSConstantTable->SetFloat( pd3dDevice, handle, fFogEnd );
			}


			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "xLightDirection");
			if ( handle )
			{
			VECT		xVectToSun;
			float		fSunAngle = A45 * 0.5f;
	 
				xVectToSun.fX = 0.0f - (float)( cosf(fSunAngle) * 500.0f );
				xVectToSun.fY = 0.0f + (float)( cosf(fSunAngle) * 500.0f ); 
				xVectToSun.fZ = 0.0f - (float)(- sinf(fSunAngle) * 500.0f );
				if ( xVectToSun.z < -70.0f )
				{
					xVectToSun.z = -70.0f;
				}
				VectNormalize( &xVectToSun );
 
				mpLandscapeShaderPSConstantTable->SetFloatArray( pd3dDevice, handle, (float*)&xVectToSun, 3 );
			}

			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "xCamPos");
			if ( handle )
			{
				mpLandscapeShaderPSConstantTable->SetFloatArray( pd3dDevice, handle, (float*)EngineCameraGetPos(), 3 );
			}

			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "gHeightRange0");
			if ( handle )
			{
				afLayerHeights[0] = afHeightValues[0];
				afLayerHeights[1] = afHeightValues[0];
				afLayerHeights[2] = afHeightValues[1];
				afLayerHeights[3] = afHeightValues[2];
				mpLandscapeShaderPSConstantTable->SetFloatArray( pd3dDevice, handle, afLayerHeights, 4 );
			}

			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "gHeightRange1");
			if ( handle )
			{
				afLayerHeights[0] = afHeightValues[1];
				afLayerHeights[1] = afHeightValues[2];
				afLayerHeights[2] = afHeightValues[3];
				afLayerHeights[3] = afHeightValues[4];
				mpLandscapeShaderPSConstantTable->SetFloatArray( pd3dDevice, handle, afLayerHeights, 4 );
			}
			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "gHeightRange2");
			if ( handle )
			{
				afLayerHeights[0] = afHeightValues[3];
				afLayerHeights[1] = afHeightValues[4];
				afLayerHeights[2] = afHeightValues[5];
				afLayerHeights[3] = afHeightValues[6];
				mpLandscapeShaderPSConstantTable->SetFloatArray( pd3dDevice, handle, afLayerHeights, 4 );
			}
			handle = mpLandscapeShaderPSConstantTable->GetConstantByName(NULL, "gHeightRange3");
			if ( handle )
			{
				afLayerHeights[0] = afHeightValues[5];
				afLayerHeights[1] = afHeightValues[6];
				afLayerHeights[2] = afHeightValues[7];
				afLayerHeights[3] = afHeightValues[7];
				mpLandscapeShaderPSConstantTable->SetFloatArray( pd3dDevice, handle, afLayerHeights, 4 );
			}
		}

		for ( nLoop = 0; nLoop < 4; nLoop++ )
		{
			EngineSetTexture( nLoop, mahLandscapeShaderSourceTextureHandles[nLoop] );

			pd3dDevice->SetTextureStageState( nLoop, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
			pd3dDevice->SetTextureStageState( nLoop, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( nLoop, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
			pd3dDevice->SetTextureStageState( nLoop, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			// Should just set all these once on init :)
			if ( InterfaceGetDeviceCaps( MAX_ANISTROPY ) > 0 )
			{
				pd3dDevice->SetSamplerState( nLoop, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);	
				pd3dDevice->SetSamplerState( nLoop, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);	
				pd3dDevice->SetSamplerState( nLoop, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);	
			}
			else
			{
				pd3dDevice->SetSamplerState( nLoop, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);	
				pd3dDevice->SetSamplerState( nLoop, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);	
				pd3dDevice->SetSamplerState( nLoop, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);	
			}
		    pd3dDevice->SetSamplerState( nLoop, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
			pd3dDevice->SetSamplerState( nLoop, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
			pd3dDevice->SetSamplerState( nLoop, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP );
		}
		EngineSetTexture( 4, mhGrassNormal );
		pd3dDevice->SetTextureStageState( 4, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		pd3dDevice->SetTextureStageState( 4, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 4, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		pd3dDevice->SetTextureStageState( 4, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	    pd3dDevice->SetSamplerState( 4, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
		pd3dDevice->SetSamplerState( 4, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
		pd3dDevice->SetSamplerState( 4, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP );
		
			// Should just set all these once on init :)
			if ( 1 ) //InterfaceGetDeviceCaps( MAX_ANISTROPY ) > 0 )
			{
				pd3dDevice->SetSamplerState( 4, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);	
				pd3dDevice->SetSamplerState( 4, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);	
				pd3dDevice->SetSamplerState( 4, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);	
			}
			else
			{
				pd3dDevice->SetSamplerState( 4, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);	
				pd3dDevice->SetSamplerState( 4, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);	
				pd3dDevice->SetSamplerState( 4, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);	
			}

		EngineResetColourMode( );
		EngineEnableFog( FALSE );
		EngineEnableLighting( FALSE );

		EngineShadersStandardVertexDeclaration(1);
		pd3dDevice->SetVertexShader(mpLandscapeShaderVertexShader);
		pd3dDevice->SetPixelShader(mpLandscapeShaderPixelShader);
		return( TRUE );
	} 
	return( FALSE );


}


/***************************************************************************
 * Function    : LandscapeShaderFreeShaders
 * 
 * Description : Release all shader related resources
 ***************************************************************************/
void		LandscapeShaderFreeShaders( void )
{
//int	nLoop;

	if ( mbLandscapeShaderHasShaders == TRUE )
	{
		SAFE_RELEASE( mpLandscapeShaderConstantTable );
		SAFE_RELEASE( mpLandscapeShaderPSConstantTable );
		SAFE_RELEASE( mpLandscapeShaderPixelShader );
		SAFE_RELEASE( mpLandscapeShaderVertexShader );

		mbLandscapeShaderHasShaders = FALSE;
	}
	mbLandscapeShaderHasInitialised = FALSE;
}


void	LandscapeShaderSetOthorgonalView( int width, int height )
{
LPDIRECT3DDEVICE9		pd3dDevice = EngineGetDXDevice();
D3DXMATRIX Ortho2D;	
D3DXMATRIX Identity;
	
	D3DXMatrixOrthoLH(&Ortho2D, (float)width, (float)height, 0.0f, 1000.0f);
	D3DXMatrixIdentity(&Identity);

	pd3dDevice->SetTransform(D3DTS_VIEW, &Identity);
	Identity._22 = -1.0f;
	Identity._41 = (float)( -(width/2) );
	Identity._42 = (float)( +(height/2) );
	pd3dDevice->SetTransform(D3DTS_WORLD, &Identity);
	pd3dDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D);

}


void		LandscapeShaderDrawSingleTile( RENDER_TILE_ITEM* pxRenderList, int nDrawX1, int nDrawY1, int nDrawX2, int nDrawY2, float fSourceTextureScale )
{
LANDRENDERVERTEX*	pxNextVertex;
LANDRENDERVERTEX*	pxBaseVertex;
int		nXMapOffset = pxRenderList->nMapX;
int		nYMapOffset = pxRenderList->nMapY;
int		nResolution = pxRenderList->nResolution;
int		nXMapOffsetPlusRes = pxRenderList->nMapX+nResolution;
int		nYMapOffsetPlusRes = pxRenderList->nMapY+nResolution;
float	fTexScale = fSourceTextureScale;
float	fUV1Mod = 0.0417f;
float	fUV2Mod = 1.1315f;//0.7351f;
LPDIRECT3DDEVICE9		pd3dDevice = EngineGetDXDevice();
uint32	ulUsageFlags = 0;
IDirect3DVertexBuffer9*	pxVertexBuffer = NULL;
LANDRENDERVERTEX*		pxLockedBuffer;
VECT	xUp = { 0.0f, 0.0f, 1.0f };

	pd3dDevice->CreateVertexBuffer( 12 * sizeof(LANDRENDERVERTEX), ulUsageFlags, D3DFVF_LANDRENDERVERTEX, D3DPOOL_DEFAULT, &pxVertexBuffer, NULL );

	fUV1Mod *= fTexScale;
	fUV2Mod *= fTexScale;

	pxVertexBuffer->Lock( 0, 0, (void**)&pxLockedBuffer, D3DLOCK_DISCARD);

	pxNextVertex = pxLockedBuffer;
	pxBaseVertex = pxNextVertex;

	*pxNextVertex = *LandscapeGetRenderVertex( nXMapOffset, nYMapOffset );
	pxNextVertex->position.x = (float)(nDrawX1);
	pxNextVertex->position.y = (float)(nDrawY1);
	pxNextVertex->tu = (float)( nXMapOffset * fUV1Mod );
	pxNextVertex->tv = (float)( nYMapOffset * fUV1Mod );
	pxNextVertex->tu2 = (float)( nXMapOffset * fUV2Mod ); 
	pxNextVertex->tv2 = (float)( nYMapOffset * fUV2Mod );
	pxNextVertex++;

	*pxNextVertex = *LandscapeGetRenderVertex( nXMapOffsetPlusRes, nYMapOffset );
	pxNextVertex->position.x = (float)(nDrawX2);
	pxNextVertex->position.y = (float)(nDrawY1);
	pxNextVertex->tu = (float)( nXMapOffsetPlusRes * fUV1Mod );
	pxNextVertex->tv = (float)( nYMapOffset * fUV1Mod );
	pxNextVertex->tu2 = (float)( nXMapOffsetPlusRes * fUV2Mod ); 
	pxNextVertex->tv2 = (float)( nYMapOffset * fUV2Mod );
	pxNextVertex++;

	*pxNextVertex = *LandscapeGetRenderVertex( nXMapOffset, nYMapOffsetPlusRes );
	pxNextVertex->position.x = (float)(nDrawX1);
	pxNextVertex->position.y = (float)(nDrawY2);
	pxNextVertex->tu = (float)( nXMapOffset * fUV1Mod );
	pxNextVertex->tv = (float)( nYMapOffsetPlusRes  * fUV1Mod );
	pxNextVertex->tu2 = (float)( nXMapOffset * fUV2Mod ); 
	pxNextVertex->tv2 = (float)( nYMapOffsetPlusRes * fUV2Mod );
	pxNextVertex++;

	*pxNextVertex = *LandscapeGetRenderVertex( nXMapOffsetPlusRes, nYMapOffset );
	pxNextVertex->position.x = (float)(nDrawX2);
	pxNextVertex->position.y = (float)(nDrawY1);
	pxNextVertex->tu = (float)( nXMapOffsetPlusRes * fUV1Mod );
	pxNextVertex->tv = (float)( nYMapOffset * fUV1Mod );
	pxNextVertex->tu2 = (float)( nXMapOffsetPlusRes * fUV2Mod ); 
	pxNextVertex->tv2 = (float)( nYMapOffset * fUV2Mod );
	pxNextVertex++;

	*pxNextVertex = *LandscapeGetRenderVertex( nXMapOffsetPlusRes, nYMapOffsetPlusRes );
	pxNextVertex->position.x = (float)(nDrawX2);
	pxNextVertex->position.y = (float)(nDrawY2);
	pxNextVertex->tu = (float)( nXMapOffsetPlusRes * fUV1Mod );
	pxNextVertex->tv = (float)( nYMapOffsetPlusRes  * fUV1Mod );
	pxNextVertex->tu2 = (float)( nXMapOffsetPlusRes * fUV2Mod ); 
	pxNextVertex->tv2 = (float)( nYMapOffsetPlusRes * fUV2Mod );
	pxNextVertex++;

	*pxNextVertex = *LandscapeGetRenderVertex( nXMapOffset, nYMapOffsetPlusRes );
	pxNextVertex->position.x = (float)(nDrawX1);
	pxNextVertex->position.y = (float)(nDrawY2);
	pxNextVertex->tu = (float)( nXMapOffset * fUV1Mod );
	pxNextVertex->tv = (float)( nYMapOffsetPlusRes  * fUV1Mod );
	pxNextVertex->tu2 = (float)( nXMapOffset * fUV2Mod ); 
	pxNextVertex->tv2 = (float)( nYMapOffsetPlusRes * fUV2Mod );

	pxVertexBuffer->Unlock();

	// TODO draw stream..
//	RenderLandscapeFlushBuffer( pxRenderList->bTexturePage, ppVertices );
	D3DPRIMITIVETYPE	primType = D3DPT_TRIANGLELIST;
	HRESULT		ret;

	pd3dDevice->SetFVF( D3DFVF_LANDRENDERVERTEX );
	ret = pd3dDevice->SetStreamSource( 0, pxVertexBuffer, 0, sizeof(LANDRENDERVERTEX) );
	ret = pd3dDevice->DrawPrimitive( primType, 0, 2 );

	pxVertexBuffer->Release();

}


#endif // ndef SERVER