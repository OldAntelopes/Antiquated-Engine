
#ifdef USING_OPENGL
#include <stdwininclude.h>
#include <gl/gl.h>
#include "../../OpenGL/InterfaceInternalsGL.h"
#else
#include "../../DirectX/InterfaceInternalsDX.h"
//#include "../../Interface Internals.h"
#endif

#include <StandardDef.h>
#include <Interface.h>

#include "../InterfaceUtil.h"
#include "Overlays.h"

 
//------------------------------------------------------------------------------------------------------------

#define		LINE_VERTEX_BUFFER_SIZE			65536


#ifndef USING_OPENGL
IGRAPHICSVERTEXBUFFER*		mpxOverlayVertexBuffer1 = NULL;
IGRAPHICSVERTEXBUFFER*		mpxOverlayVertexBuffer2 = NULL;
int							mnNextOverlayVertexIndex = 0;
int							mnNextOverlayVertexIndex2 = 0;
BOOL	mboOverlayBuffer2Locked = FALSE;
BOOL	mboOverlayBuffer1Locked = FALSE;

FLATVERTEX* mpOverlayVertices1 = NULL;
FLATVERTEX*	mpOverlayVertices2 = NULL;
FLATVERTEX*	mpIconVertices = NULL;

IGRAPHICSVERTEXBUFFER*		mpxLineVertexBuffer = NULL;
FLATVERTEX*	mpLineVertices = NULL;
int			mnNextLineVertex = 0;
#endif


#ifndef USING_OPENGL

//------------------------------------------------------------------------------------------------------------

void	RenderLinesBuffer( void )
{
int		nDrawHowMany;

	if ( mpLineVertices != NULL )
	{
		mpxLineVertexBuffer->Unlock();
		mpLineVertices = NULL;
	}

	nDrawHowMany = (mnNextLineVertex / 2);

	if ( nDrawHowMany > 0 )
	{
	D3DXMATRIX Ortho2D;	
	D3DXMATRIX Identity;
	
		D3DXMatrixOrthoLH(&Ortho2D, (float)InterfaceGetWidth(), (float)InterfaceGetHeight(), 0.0f, 1.0f);
		D3DXMatrixIdentity(&Identity);

		mpInterfaceD3DDevice->SetTransform(D3DTS_VIEW, &Identity);
		Identity._22 = -1.0f;
		Identity._41 = (float)( -(InterfaceGetWidth()/2) );
		Identity._42 = (float)( +(InterfaceGetHeight()/2) );
		mpInterfaceD3DDevice->SetTransform(D3DTS_WORLD, &Identity);
		mpInterfaceD3DDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D);
		mpInterfaceD3DDevice->SetVertexShader( D3DFVF_FLATVERTEX );
		mpInterfaceD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
		mpInterfaceD3DDevice->SetTexture( 0, NULL );
		mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
		mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
		mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
		mpInterfaceD3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
		mpInterfaceD3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
		mpInterfaceD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

		mpInterfaceD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
		if ( mboRenderLineAlpha == TRUE )
		{
			mpInterfaceD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			mpInterfaceD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			mpInterfaceD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
		else
		{
			mpInterfaceD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		}
		mpInterfaceD3DDevice->SetStreamSource( 0, mpxLineVertexBuffer, sizeof(FLATVERTEX) );
		mpInterfaceD3DDevice->DrawPrimitive( D3DPT_LINELIST, 0, nDrawHowMany );
		
		mnNextLineVertex = 0;
	}
}


INTERFACE_API void InterfaceLine( int nLayer, int nX1, int nY1, int nX2, int nY2, uint32 ulCol1, uint32 ulCol2 )
{
FLATVERTEX*		pxLineVertex;
int		nWidth = InterfaceGetWidth();
int		nHeight = InterfaceGetHeight();

	if ( (( nX1 < 0 ) && ( nX2 < 0 )) ||
		 (( nY1 < 0 ) && ( nY2 < 0 )) ||
		 (( nX1 > nWidth ) && ( nX2 > nWidth )) ||
		 (( nY1 > nHeight ) && ( nX2 > nHeight )) )
	{
		return;
	}

	if ( mnNextLineVertex >= (LINE_VERTEX_BUFFER_SIZE-2) )
	{
		RenderLinesBuffer();
	}

	if ( mpLineVertices == NULL )
	{
		if( ( mpxLineVertexBuffer ) && 
			( FAILED( mpxLineVertexBuffer->Lock(0, 0, (BYTE**)&mpLineVertices, D3DLOCK_DISCARD)) ) )
		{
			return;
		}
		mnNextLineVertex = 0;
	}

	pxLineVertex = mpLineVertices + mnNextLineVertex;

	pxLineVertex->color = ulCol1;
//	mpLineVertices[mnNextLineVertex].tu = 0.0f;
//	mpLineVertices[mnNextLineVertex].tv = 0.0f;
	pxLineVertex->x = (float)nX1 + mnInterfaceDrawX;
	pxLineVertex->y = (float)nY1 + mnInterfaceDrawY;
	pxLineVertex->z = 0.0f;
	pxLineVertex++;

	pxLineVertex->color = ulCol2;
	pxLineVertex->x = (float)nX2 + mnInterfaceDrawX;
	pxLineVertex->y = (float)nY2 + mnInterfaceDrawY;
	pxLineVertex->z = 0.0f;
	
	mnNextLineVertex += 2;

}

/***************************************************************************
 * Function    : AddOverlayVertices
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
int AddOverlayVertices( FLATVERTEX* pVertices, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, int nVertIndex, int nIconTextureNum )
{
float	fX;
float	fY;
float	fWidth;
float	fHeight;
float	fV;
float	fV1;

	fV = (float)( nIconTextureNum ) / 8.0f;
	fV1 = fV + ( 1.0f / 8.0f );

	fX = (FLOAT)( nX );
	fY = (FLOAT)( nY );
	fWidth = (FLOAT)( nWidth );
	fHeight = (FLOAT)( nHeight );


	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	return( nVertIndex );

}
#endif

/***************************************************************************
 * Function    : AddOverlayRect
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void AddOverlayRect( int nX, int nY, int nWidth, int nHeight, uint32 ulCol )
{
#ifdef USING_OPENGL

#else
	if ( PANIC_IF( mnNextOverlayVertexIndex2 >= (NUM_OVERLAY_VERTICES-6), "Too many secondary overlay boxes added" ) == TRUE )
	{
		return;
	}

	if ( !mboOverlayBuffer1Locked )
	{
		if( FAILED( mpxOverlayVertexBuffer1->Lock( 0, 0, (BYTE**)&mpOverlayVertices1, D3DLOCK_DISCARD ) ) )
		{
			return;
		}
		mboOverlayBuffer1Locked = TRUE;
	}
	
	mnNextOverlayVertexIndex = AddOverlayVertices( mpOverlayVertices1, nX + mnInterfaceDrawX, nY + mnInterfaceDrawY, nWidth, nHeight, ulCol, mnNextOverlayVertexIndex, 0 );
#endif
}



/***************************************************************************
 * Function    : AddOverlayRect2
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void AddOverlayRect2( int nX, int nY, int nWidth, int nHeight, uint32 ulCol )
{
#ifdef USING_OPENGL
float	fX;
float	fY;
float	fWidth;
float	fHeight;
float	fR, fG, fB, fA;

	fX = (FLOAT)( nX );
	fY = (FLOAT)( nY );
	fWidth = (FLOAT)( nWidth );
	fHeight = (FLOAT)( nHeight );

	// TEMP - Should be adding to drawlist
    glDisable (GL_TEXTURE_2D); /* disable texture mapping */

	glBegin (GL_QUADS);

	InterfaceUnpackCol( ulCol, &fR, &fG, &fB, &fA );
	glColor4f( fR, fG, fB, fA );
	glVertex3f (fX, fY, 0.0f);
	glVertex3f (fX + fWidth, fY, 0.0f);
	glVertex3f (fX + fWidth, fY + fHeight, 0.0f);
	glVertex3f (fX, fY + fHeight, 0.0f);

	glEnd ();
 
#else
	if ( PANIC_IF( mnNextOverlayVertexIndex2 >= (NUM_OVERLAY_VERTICES-6), "Too many secondary overlay boxes added" ) == TRUE )
	{
		return;
	}
    
	if ( mpxOverlayVertexBuffer2 )
	{
		if ( mboOverlayBuffer2Locked == FALSE )
		{
			mboOverlayBuffer2Locked = TRUE;
			if( FAILED( mpxOverlayVertexBuffer2->Lock( 0, 0, (BYTE**)&mpOverlayVertices2, D3DLOCK_DISCARD ) ) )
			{
				return;
			}
		}

		mnNextOverlayVertexIndex2 = AddOverlayVertices( mpOverlayVertices2, nX + mnInterfaceDrawX, nY + mnInterfaceDrawY, nWidth, nHeight, ulCol, mnNextOverlayVertexIndex2, 0 );
	}
#endif
}



INTERFACE_API void InterfaceShadedRect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol1, uint32 ulCol2,uint32 ulCol3, uint32 ulCol4 )
{
float	fX;
float	fY;
float	fWidth;
float	fHeight;
float	fR, fG, fB, fA;

	fX = (FLOAT)( nX );
	fY = (FLOAT)( nY );
	fWidth = (FLOAT)( nWidth );
	fHeight = (FLOAT)( nHeight );

#ifdef USING_OPENGL
	
	// TEMP - Should be adding to drawlist
    glDisable (GL_TEXTURE_2D); /* disable texture mapping */

	glBegin (GL_QUADS);

	InterfaceUnpackCol( ulCol1, &fR, &fG, &fB, &fA );
	glColor4f( fR, fG, fB, fA );
	glVertex3f (fX, fY, 0.0f);
	InterfaceUnpackCol( ulCol2, &fR, &fG, &fB, &fA );
	glColor4f( fR, fG, fB, fA );
	glVertex3f (fX + fWidth, fY, 0.0f);
	InterfaceUnpackCol( ulCol3, &fR, &fG, &fB, &fA );
	glColor4f( fR, fG, fB, fA );
	glVertex3f (fX + fWidth, fY + fHeight, 0.0f);
	InterfaceUnpackCol( ulCol4, &fR, &fG, &fB, &fA );
	glColor4f( fR, fG, fB, fA );
	glVertex3f (fX, fY + fHeight, 0.0f);

	glEnd ();

#else
FLATVERTEX* pVertices;
int nVertIndex;
float	fV = 0.0f;
float	fV1 = 1.0f;

	if ( PANIC_IF( mnNextOverlayVertexIndex >= (NUM_OVERLAY_VERTICES-6), "Too many overlay boxes added" ) == TRUE )
	{
		return;
	}

	if ( !mboOverlayBuffer1Locked )
	{
		if( FAILED( mpxOverlayVertexBuffer1->Lock( 0, 0, (BYTE**)&mpOverlayVertices1, D3DLOCK_DISCARD ) ) )
		{
			return;
		}
		mboOverlayBuffer1Locked = TRUE;
	}

	nVertIndex = mnNextOverlayVertexIndex;
	pVertices = mpOverlayVertices1;
	

	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol1;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol2;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol4;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol3;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX;
	pVertices[ nVertIndex ].y = fY;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol1;
	pVertices[ nVertIndex ].tu       = 0.0f;
	pVertices[ nVertIndex ].tv       = fV;
	nVertIndex++;

	pVertices[ nVertIndex ].x = fX + fWidth;
	pVertices[ nVertIndex ].y = fY + fHeight;
	pVertices[ nVertIndex ].z = 0.0f;
//	pVertices[ nVertIndex ].rhw = 1.0f;
	pVertices[ nVertIndex ].color    = ulCol4;
	pVertices[ nVertIndex ].tu       = 1.0f;
	pVertices[ nVertIndex ].tv       = fV1;
	nVertIndex++;

	mnNextOverlayVertexIndex = nVertIndex;
#endif
}

/***************************************************************************
 * Function    : InterfaceRect
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceRect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol)
{
	if ( nLayer == 0 )
	{
		AddOverlayRect( nX, nY, nWidth, nHeight, ulCol );
	}
	else
	{
		AddOverlayRect2( nX, nY, nWidth, nHeight, ulCol );
	}
}

/***************************************************************************
 * Function    : AddBox
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceOutlineBox ( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol )
{
	AddOverlayRect( nX - 2, nY - 2, 2, nHeight + 4, ulCol );
	AddOverlayRect( nX + nWidth, nY - 2, 2, nHeight + 4, ulCol );

	AddOverlayRect( nX, nY - 2, nWidth, 2, ulCol );
	AddOverlayRect( nX, nY + nHeight, nWidth, 2, ulCol );

}
/***************************************************************************
 * Function    : AddBox2
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
INTERFACE_API void InterfaceShadedBox( int nLayer, int nX, int nY, int nWidth, int nHeight, int nStyle )
{
	if ( ( nStyle < 0xFFFFFF ) &&
		 ( nStyle > 0 ) )
	{
		switch ( nStyle )
		{
		case 0:
		default:
			AddOverlayRect2( nX - 2, nY - 2, 2, nHeight + 4, 0x404080CF );
			AddOverlayRect2( nX + nWidth, nY - 2, 2, nHeight + 4, 0x404080CF );

			AddOverlayRect2( nX, nY - 2, nWidth, 2, 0x404080CF );
			AddOverlayRect2( nX, nY + nHeight, nWidth, 2, 0x404080CF );
			break;
		case 1:
			AddOverlayRect2( nX, nY, nWidth, nHeight, 0x70000000 );
			AddOverlayRect2( nX - 2, nY - 2, 2, nHeight + 4, 0x404080CF );
			AddOverlayRect2( nX + nWidth, nY - 2, 2, nHeight + 4, 0x404080CF );

			AddOverlayRect2( nX, nY - 2, nWidth, 2, 0x404080CF );
			AddOverlayRect2( nX, nY + nHeight, nWidth, 2, 0x404080CF );
			break;
		case 2:
			AddOverlayRect2( nX, nY, nWidth, nHeight, 0x70404040 );
			AddOverlayRect2( nX - 2, nY - 2, 2, nHeight + 4, 0x60B0B0B0 );
			AddOverlayRect2( nX + nWidth, nY - 2, 2, nHeight + 4, 0x60B0B0B0 );

			AddOverlayRect2( nX, nY - 2, nWidth, 2, 0x60101010 );
			AddOverlayRect2( nX, nY + nHeight, nWidth, 2, 0x60101010 );
			break;
		}
	}
	else 
	{
		AddOverlayRect2( nX, nY, nWidth, nHeight, (uint32)(nStyle) );

		AddOverlayRect2( nX - 2, nY - 2, 2, nHeight + 4, 0x404080CF );
		AddOverlayRect2( nX + nWidth, nY - 2, 2, nHeight + 4, 0x404080CF );

		AddOverlayRect2( nX, nY - 2, nWidth, 2, 0x404080CF );
		AddOverlayRect2( nX, nY + nHeight, nWidth, 2, 0x404080CF );
	}

}

BOOL	mbAdditiveOverlays = FALSE;

INTERFACE_API void	InterfaceOverlaysAdditive( BOOL bFlag )
{
	mbAdditiveOverlays = bFlag;
}

/***************************************************************************
 * Function    : RenderOverlays
 * Params      : 
 * Returns     :
 * Description : 
 ***************************************************************************/
void RenderOverlays( int nLayer )
{
#ifdef USING_OPENGL

#else
int				nDrawHowMany;

	if ( nLayer == 1 )
	{
		if ( mnNextLineVertex >= 0 )
		{
			RenderLinesBuffer();
		}
		nDrawHowMany = (mnNextOverlayVertexIndex2 / 3);
	}
	else
	{
		nDrawHowMany = (mnNextOverlayVertexIndex / 3);
	}

	if ( nDrawHowMany > 0 )
	{
		D3DXMATRIX Ortho2D;	
		D3DXMATRIX Identity;
		D3DXMatrixOrthoLH(&Ortho2D, (float)InterfaceGetWidth(), (float)InterfaceGetHeight(), 0.0f, 1.0f);
		D3DXMatrixIdentity(&Identity);

		mpInterfaceD3DDevice->SetTransform(D3DTS_VIEW, &Identity);
		Identity._22 = -1.0f;
		Identity._41 = (float)-(InterfaceGetWidth()/2);
		Identity._42 = (float)+(InterfaceGetHeight()/2);
		mpInterfaceD3DDevice->SetTransform(D3DTS_WORLD, &Identity);
		mpInterfaceD3DDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D);

		mpInterfaceD3DDevice->SetVertexShader( D3DFVF_FLATVERTEX );
		mpInterfaceD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
		mpInterfaceD3DDevice->SetTexture( 0, NULL );
		mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	//	mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	//	mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		mpInterfaceD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	
		// Enable alpha testing (skips pixels with less than a certain alpha.)
		mpInterfaceD3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
		mpInterfaceD3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
		mpInterfaceD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		mpInterfaceD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
		mpInterfaceD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		if ( mbAdditiveOverlays )
		{
			mpInterfaceD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			mpInterfaceD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
		}
		else
		{
			mpInterfaceD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			mpInterfaceD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}

		if ( nLayer == 0 )
		{
			if ( mboOverlayBuffer1Locked )
			{
				mpxOverlayVertexBuffer1->Unlock();
				mboOverlayBuffer1Locked = FALSE;
			}
			mpInterfaceD3DDevice->SetStreamSource( 0, mpxOverlayVertexBuffer1, sizeof(FLATVERTEX) );
			mpInterfaceD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, nDrawHowMany );
			mnNextOverlayVertexIndex = 0;
		}
		else
		{
			if ( mboOverlayBuffer2Locked )
			{
				mpxOverlayVertexBuffer2->Unlock();
				mboOverlayBuffer2Locked = FALSE;
			}
			mpInterfaceD3DDevice->SetStreamSource( 0, mpxOverlayVertexBuffer2, sizeof(FLATVERTEX) );
			mpInterfaceD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, nDrawHowMany );
			mnNextOverlayVertexIndex2 = 0;
		}
	}
#endif
}


/***************************************************************************
 * Function    : InitialiseOverlays
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
HRESULT InitialiseOverlays( void )
{
#ifdef USING_OPENGL

#else
	if ( mpxOverlayVertexBuffer1 == NULL )
	{
		// Create the vertex buffer.
		if( FAILED( mpInterfaceD3DDevice->CreateVertexBuffer( NUM_OVERLAY_VERTICES * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  D3DPOOL_DEFAULT, &mpxOverlayVertexBuffer1 ) ) )
		{
			PANIC_IF( TRUE, "Couldnt create Overlay Vertex buffer 1");
			return ( FALSE);
		}

		if( FAILED( mpInterfaceD3DDevice->CreateVertexBuffer( NUM_OVERLAY_VERTICES * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  D3DPOOL_DEFAULT, &mpxOverlayVertexBuffer2 ) ) )
		{
			PANIC_IF( TRUE, "Couldnt create Overlay Vertex buffer 2");
			return ( FALSE);
		}
	}

	if ( mpxLineVertexBuffer == NULL )
	{
		// Create the vertex buffer.
		if( FAILED( mpInterfaceD3DDevice->CreateVertexBuffer( LINE_VERTEX_BUFFER_SIZE * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  D3DPOOL_DEFAULT, &mpxLineVertexBuffer ) ) )
		{
			PANIC_IF( TRUE, "Couldnt create Line Vertex buffer 1");
			return ( FALSE);
		}
	}
#endif

	return S_OK;

} /** End of function InitialiseOverlays **/


/***************************************************************************
 * Function    : FreeOverlays
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void FreeOverlays( void )
{
#ifdef USING_OPENGL

#else
    if( mpxLineVertexBuffer != NULL )
	{
        mpxLineVertexBuffer->Release();
		mpxLineVertexBuffer = NULL;
	}

    if( mpxOverlayVertexBuffer1 != NULL )
	{
		if ( mboOverlayBuffer1Locked )
		{
			mpxOverlayVertexBuffer1->Unlock();
			mboOverlayBuffer1Locked = FALSE;
		}
        mpxOverlayVertexBuffer1->Release();
		mpxOverlayVertexBuffer1 = NULL;
	}

    if( mpxOverlayVertexBuffer2 != NULL )
	{
		if ( mboOverlayBuffer2Locked )
		{
			mpxOverlayVertexBuffer2->Unlock();
			mboOverlayBuffer2Locked = FALSE;
		}

        mpxOverlayVertexBuffer2->Release();
		mpxOverlayVertexBuffer2 = NULL;
	}
#endif

} 


/***************************************************************************
 * Function    : LockOverlays
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void LockOverlays( void )
{
#ifdef USING_OPENGL

#else
    if( FAILED( mpxOverlayVertexBuffer1->Lock( 0, 0, (BYTE**)&mpOverlayVertices1, D3DLOCK_DISCARD ) ) )
	{
        return;
	}
	mboOverlayBuffer1Locked = TRUE;

    if( FAILED( mpxOverlayVertexBuffer2->Lock( 0, 0, (BYTE**)&mpOverlayVertices2, D3DLOCK_DISCARD ) ) )
	{
        return;
	}
	mboOverlayBuffer2Locked = TRUE;
#endif
}



/***************************************************************************
 * Function    : UnlockOverlays
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void UnlockOverlays( void )
{
#ifdef USING_OPENGL

#else
	mpxOverlayVertexBuffer1->Unlock();
	mpxOverlayVertexBuffer2->Unlock();
	mboOverlayBuffer1Locked = FALSE;
	mboOverlayBuffer2Locked = FALSE;
#endif
}

#ifdef USING_OPENGL
INTERFACE_API void	InterfaceTri( int nLayer, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, uint32 ulCol1, uint32 ulCol2, uint32 ulCol3 )
{
	// todo
}

INTERFACE_API void InterfaceLine( int nLayer, int nX1, int nY1, int nX2, int nY2, uint32 ulCol1, uint32 ulCol2 )
{
	// todo
}

#endif