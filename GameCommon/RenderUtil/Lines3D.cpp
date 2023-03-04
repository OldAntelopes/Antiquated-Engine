
#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#include "Lines3D.h"


VERTEX_BUFFER_HANDLE		mhLineVertexBuffer = NOTFOUND;
int							mnNextLineVert = 0;
int							mnMaxLineVertices = 32768;

BOOL	mbLineVertexBufferLocked = FALSE;


/***************************************************************************
 * Function    : Lines3DAddLine
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void Lines3DAddLine( const VECT* pxPoint1, const VECT* pxPoint2, ulong ulCol, ulong ulCol2 )
{
ENGINEBUFFERVERTEX* pVertices;

	if ( mhLineVertexBuffer == NOTFOUND )
	{
		return;
	}
	if ( mnNextLineVert >= mnMaxLineVertices - 2 )
	{
		return;
	}

	if ( mbLineVertexBufferLocked == FALSE )
	{	
		if( EngineVertexBufferLock( mhLineVertexBuffer, TRUE ) == FALSE )
		{
			return;
		}
		mbLineVertexBufferLocked = TRUE;
	}

	pVertices = EngineVertexBufferGetBufferPointer(mhLineVertexBuffer, 2 );

	if ( pVertices )
	{
		// Fill the vertex buffer. We are setting the tu and tv texture
		// coordinates, which range from 0.0 to 1.0
		pVertices->position = *pxPoint1;
		pVertices->color    = ulCol;
		pVertices->tu       = 0.0f;
		pVertices->tv       = 0.0f;
		pVertices++;

		pVertices->position = *pxPoint2;
		pVertices->color    = ulCol2;
		pVertices->tu       = 0.0f;
		pVertices->tv       = 0.0f;
		mnNextLineVert += 2;
	}
}


/***************************************************************************
 * Function    : Lines3DFlush
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void Lines3DFlush( LINES3D_RENDER_PARAMS* pxRenderParams  )
{
int		nDrawHowMany;
LINES3D_RENDER_PARAMS		xParams;

	if ( pxRenderParams )
	{
		xParams = *pxRenderParams;
	}
	else
	{
		xParams.boAdditive = FALSE;
		xParams.boFog = FALSE;
		xParams.boZWrite = FALSE;
	}

	if ( mbLineVertexBufferLocked == TRUE )
	{	
		EngineVertexBufferUnlock( mhLineVertexBuffer );
		mbLineVertexBufferLocked = FALSE;
	}

	nDrawHowMany = (mnNextLineVert / 2);
	if ( nDrawHowMany > 0 )
	{
		if ( mhLineVertexBuffer == NOTFOUND )
		{
			mnNextLineVert = 0;
			return;
		}

//		EngineSetVertexFormat( VERTEX_FORMAT_NORMAL );

	    EngineEnableZTest( TRUE );
//	    EngineEnableZTest( FALSE );
	    EngineEnableZWrite( xParams.boZWrite );
	    EngineEnableLighting( FALSE );
		EngineEnableFog( xParams.boFog );
		EngineEnableCulling( 0 );
//		EngineSetZBias( 2 );
		EngineEnableBlend( TRUE );
		if ( xParams.boAdditive == TRUE )
		{
			EngineSetBlendMode( BLEND_MODE_SRCALPHA_ADDITIVE );
		}
		else
		{
			EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
		}
		EngineSetColourMode( 0, COLOUR_MODE_DIFFUSE_ONLY );
		EngineSetTexture( 0, 0 );

		EngineVertexBufferRender( mhLineVertexBuffer, LINE_LIST );
		EngineVertexBufferReset( mhLineVertexBuffer );

//		EngineSetZBias( 0 );
		EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );

	}
	mnNextLineVert = 0;

} 



/***************************************************************************
 * Function    : Lines3DInitialise
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void Lines3DInitialise( int nMaxVertices )
{
	if ( mhLineVertexBuffer == NOTFOUND )
	{
		mnMaxLineVertices = nMaxVertices;
		mhLineVertexBuffer = EngineCreateVertexBuffer( mnMaxLineVertices, 0, "Lines3d" );
	    // Create the vertex buffer.
	    if( mhLineVertexBuffer == NOTFOUND )
	    {
			PANIC_IF( TRUE, "Couldnt create Line Vertex buffer");
			return;
	    }
	}

} 


/***************************************************************************
 * Function    : Lines3DShutdown
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void Lines3DShutdown( void )
{

    if( mhLineVertexBuffer != NOTFOUND )
	{
		EngineVertexBufferFree( mhLineVertexBuffer );
		mhLineVertexBuffer = NOTFOUND;
	}


} 
