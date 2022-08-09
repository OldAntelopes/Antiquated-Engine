#include <d3dx9.h>
#include <Engine.h>


#include <StandardDef.h>
#include <Rendering.h>
#include <Interface.h>

#include "Point Renderer.h"


IDirect3DVertexBuffer9*		mpxPointVertexBuffer = NULL;
int							mnNextPointVert = 0;
 
BOOL				msbPointBufferLocked = FALSE;
CUSTOMVERTEX*		mspPointBufferLockedVertices = NULL;

/***************************************************************************
 * Function    : AddLine
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void PointRenderAdd( MVECT* pxPoint1, ulong ulCol )
{

#ifndef STANDALONE
	if ( FrontEndGetOptions()->bDisablePointRender == 1 ) return;
#endif

	if ( mpxPointVertexBuffer == NULL )
	{
		return;
	}

	if ( msbPointBufferLocked == FALSE )
	{
		// Fill the vertex buffer. We are setting the tu and tv texture
		// coordinates, which range from 0.0 to 1.0
		if( FAILED( mpxPointVertexBuffer->Lock( 0, 0, (void**)&mspPointBufferLockedVertices, D3DLOCK_DISCARD ) ) )
		{
			return;
		}
		msbPointBufferLocked = TRUE;
	}

	mspPointBufferLockedVertices[ mnNextPointVert ].position = *pxPoint1;
	mspPointBufferLockedVertices[ mnNextPointVert ].color    = ulCol;
	mspPointBufferLockedVertices[ mnNextPointVert ].tu       = 0.0f;
	mspPointBufferLockedVertices[ mnNextPointVert ].tv       = 0.0f;
	mnNextPointVert++;

	if ( mnNextPointVert >= MAX_POINTS )
	{
		mpxPointVertexBuffer->Unlock();
		mspPointBufferLockedVertices = NULL;
		msbPointBufferLocked = FALSE;

		RenderPoints( 3.0f, FALSE, FALSE );

		mnNextPointVert = 0;
	}



}


/***************************************************************************
 * Function    : RenderLines
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void RenderPoints( float fPointSize, BOOL boFog, BOOL boZWrite )
{
int		nDrawHowMany;

	if ( msbPointBufferLocked == TRUE )
	{
		mpxPointVertexBuffer->Unlock();
		mspPointBufferLockedVertices = NULL;
		msbPointBufferLocked = FALSE;
	}

	nDrawHowMany = mnNextPointVert;
	if ( nDrawHowMany > 0 )
	{
		EngineSetVertexFormat( VERTEX_FORMAT_NORMAL );
	    EngineEnableZTest( TRUE );
	    EngineEnableZWrite( boZWrite);
	    EngineEnableLighting( FALSE );
 		EngineEnableFog( boFog );
 		EngineEnableBlend( TRUE );
 		EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
		EngineSetColourMode( 0, COLOUR_MODE_DIFFUSE_ONLY );
		EngineSetTexture( 0,0 );
		g_pd3dDevice->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE );
		g_pd3dDevice->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&fPointSize));

		g_pd3dDevice->SetStreamSource( 0, mpxPointVertexBuffer, 0, sizeof(CUSTOMVERTEX) );
	    g_pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, 0, nDrawHowMany );
	}
	mnNextPointVert = 0;

} 



/***************************************************************************
 * Function    : InitialisePointRenderer
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void InitialisePointRenderer( void )
{
	if ( mpxPointVertexBuffer == NULL )
	{
		// Create the vertex buffer.
		if( FAILED( g_pd3dDevice->CreateVertexBuffer( MAX_POINTS * sizeof(CUSTOMVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_CUSTOMVERTEX,
													  D3DPOOL_DEFAULT, &mpxPointVertexBuffer, NULL ) ) )
		{
			PANIC_IF( TRUE, "Couldnt create Point Vertex buffer");
			return;
		}
	}
	mnNextPointVert = 0;

} 


/***************************************************************************
 * Function    : FreeLineRenderer
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void FreePointRenderer( void )
{

    if( mpxPointVertexBuffer != NULL )
	{
        mpxPointVertexBuffer->Release();
		mpxPointVertexBuffer = NULL;
	}


} 
