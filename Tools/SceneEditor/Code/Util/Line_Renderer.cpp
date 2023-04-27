
#include <d3dx9.h>
#include <Engine.h>

#include <StandardDef.h>
#include <Rendering.h>
#include <Interface.h>

#include "Line Renderer.h"


IDirect3DVertexBuffer9*		mpxLineVertexBuffer = NULL;
int							mnNextLineVert = 0;
int			mnMaxLineVertices = 32768;


/***************************************************************************
 * Function    : AddLine
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void AddLine( MVECT* pxPoint1, MVECT* pxPoint2, uint32 ulCol, uint32 ulCol2 )
{
CUSTOMVERTEX* pVertices;

	if ( mpxLineVertexBuffer == NULL )
	{
		return;
	}
	if ( mnNextLineVert >= mnMaxLineVertices - 2 )
	{
		return;
	}

    // Fill the vertex buffer. We are setting the tu and tv texture
    // coordinates, which range from 0.0 to 1.0
    if( FAILED( mpxLineVertexBuffer->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD) ) )
	{
        return;
	}

	pVertices[ mnNextLineVert ].position = *pxPoint1;
	pVertices[ mnNextLineVert ].color    = ulCol;
	pVertices[ mnNextLineVert ].tu       = 0.0f;
	pVertices[ mnNextLineVert ].tv       = 0.0f;
	mnNextLineVert++;

	pVertices[ mnNextLineVert ].position = *pxPoint2;
	pVertices[ mnNextLineVert ].color    = ulCol2;
	pVertices[ mnNextLineVert ].tu       = 0.0f;
	pVertices[ mnNextLineVert ].tv       = 0.0f;
	mnNextLineVert++;

	mpxLineVertexBuffer->Unlock();


}


/***************************************************************************
 * Function    : RenderLines
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void RenderLines( BOOL boCamChange, BOOL boFog, BOOL boAdditive, BOOL boZWrite )
{
int		nDrawHowMany;
LPDIRECT3DDEVICE9		pxD3DDevice = EngineGetDXDevice();

	if ( boCamChange == TRUE )
	{
#ifdef TOOL
D3DXMATRIX	matWorld;
		// Set the world matrix as identity
		D3DXMatrixIdentity( &matWorld );	
		pxD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
#else
		CameraApplyCurrentToView();
#endif
	}

	nDrawHowMany = (mnNextLineVert / 2);
	if ( nDrawHowMany > 0 )
	{
		EngineSetVertexFormat( VERTEX_FORMAT_NORMAL );

	    EngineEnableZTest( TRUE );
	    EngineEnableZWrite( boZWrite);
	    EngineEnableLighting( FALSE );
		EngineEnableFog( boFog );
#ifdef TOOL
		EngineEnableCulling( 0 );
#else
		if ( GetGameStage() == GAME_STAGE_MAP_EDITOR )
		{
			EngineSetZBias( 2 );
			EngineEnableFog( FALSE );
			boAdditive = FALSE;
		}
#endif
		EngineEnableBlend( TRUE );
		if ( boAdditive == TRUE )
		{
			EngineSetBlendMode( BLEND_MODE_SRCALPHA_ADDITIVE );
		}
		else
		{
			EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
		}
		EngineSetColourMode( 0, COLOUR_MODE_DIFFUSE_ONLY );
		EngineSetTexture( 0, 0 );

		if ( mpxLineVertexBuffer == NULL )
		{
			mnNextLineVert = 0;
			return;
		}

		pxD3DDevice->SetStreamSource( 0, mpxLineVertexBuffer, 0, sizeof(CUSTOMVERTEX) );
	    if ( pxD3DDevice->DrawPrimitive( D3DPT_LINELIST, 0, nDrawHowMany ) != D3D_OK )
		{
#ifdef TOOL
			PANIC_IF( TRUE, "Line render failed" );
#endif
		}
#ifndef TOOL
		if ( GetGameStage() == GAME_STAGE_MAP_EDITOR )
		{
			EngineSetZBias( 0 );
		}
#endif

	}
	mnNextLineVert = 0;

} /** End of function RenderLines **/



/***************************************************************************
 * Function    : InitialiseLineRenderer
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void InitialiseLineRenderer( int nMaxVertices )
{
LPDIRECT3DDEVICE9		pxD3DDevice = EngineGetDXDevice();

	mnMaxLineVertices = nMaxVertices;
	if ( mpxLineVertexBuffer == NULL )
	{
	    // Create the vertex buffer.
	    if( FAILED( pxD3DDevice->CreateVertexBuffer( mnMaxLineVertices * sizeof(CUSTOMVERTEX),
		                                              D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_CUSTOMVERTEX,
			                                          D3DPOOL_DEFAULT, &mpxLineVertexBuffer, NULL  ) ))
	    {
			PANIC_IF( TRUE, "Couldnt create Line Vertex buffer");
			return;
	    }
	}

} /** End of function InitialiseLineRenderer **/


/***************************************************************************
 * Function    : FreeLineRenderer
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void FreeLineRenderer( void )
{

    if( mpxLineVertexBuffer != NULL )
	{
        mpxLineVertexBuffer->Release();
		mpxLineVertexBuffer = NULL;
	}


} /** End of function FreeLineRenderer **/
