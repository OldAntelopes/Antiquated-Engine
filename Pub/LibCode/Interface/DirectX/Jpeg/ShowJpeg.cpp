
#ifdef TUD9

#include "../InterfaceInternalsDX.h"

#include <stdio.h>

#include <StandardDef.h>
#include <Interface.h>

#include "../LibCode/CodeUtil/Archive.h"

#include "ShowJpeg.h"
#include "Jpegfile.h"

typedef struct
{
	POINT	xPoint;

	BOOL	boDisplayThisFrame;

	int		nFlags;

} JPEG_DRAW_INFO;

typedef struct
{
	byte*	pbSource;
	byte*	pbDest;

	int		nDestFormat;
	int		nDestPitch;
	int		nSrcPitch;

	int		nX;
	int		nY;

	int		nWidth;
	int		nHeight;

	int		nFlags;

} COPY_RECT_INFO;


typedef struct
{
	int		nLayer;
	IGRAPHICSSURFACE*		pxTexture;
	int		nX;
	int		nY;
	int		nWidth;
	int		nHeight;
	int		nFlags;

} JPEG_DRAW_BUFFER;

#define		MAX_BUFFERED_JPEGS		8

JPEG_DRAW_BUFFER		maxBufferedJpegs[ MAX_BUFFERED_JPEGS ];

int		mnNextBufferedJpeg = 0;

IGRAPHICSSURFACE*		mpxJpegSurface = NULL;
IGRAPHICSSURFACE*		mpxBackBufferCopySurface = NULL;
RECT					mxJpegRect;
char			mszLastJpegFile[256] = { 0 };;

JPEG_DRAW_INFO	mxJpegDraw;



/***************************************************************************
 * Function    : CopyLine555Additive
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
inline void CopyLine555Additive ( ushort* puwWrite, byte* pbRead, int nWidth )
{
int	nPixLoop;
ushort	uwPixel;
byte		bR;
byte		bG;
byte		bB;
byte		bR2;
byte		bG2;
byte		bB2;
int		nRes;

	nPixLoop = 0;
	do
	{
		bR = *(pbRead++);
		bG = *(pbRead++);
		bB = *(pbRead++);
		uwPixel = *(puwWrite);
		bR2 = ((uwPixel >> 10)<<3);
		bG2 = (((uwPixel >> 5) & 0x1F)<<3);
		bB2 = ((uwPixel & 0x1F )<<3);

		nRes = bR2 + bR;
		if ( nRes > 255 ) { bR = 255; }	else { bR = (byte)( nRes ); }
		nRes = bG2 + bG;
		if ( nRes > 255 ) { bG = 255; }	else { bG = (byte)( nRes ); }
		nRes = bB2 + bB;
		if ( nRes > 255 ) { bB = 255; }	else { bB = (byte)( nRes ); }

		uwPixel = ((bR >> 3)<< 10) | ((bG>>3)<<5) | (bB>>3);
		*(puwWrite++) = uwPixel ;

		nPixLoop++;
	} while( nPixLoop < nWidth );
}


/***************************************************************************
 * Function    : CopyLine888Additive
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
inline void CopyLine888Additive ( ulong* pulWrite, byte* pbRead, int nWidth )
{
int	nPixLoop;
ulong	ulPixel;
byte		bR;
byte		bG;
byte		bB;
byte		bR2;
byte		bG2;
byte		bB2;
int		nRes;

	nPixLoop = 0;
	do
	{
		bR = *(pbRead++);
		bG = *(pbRead++);
		bB = *(pbRead++);
		ulPixel = *(pulWrite);
		bR2 = (byte)(ulPixel >> 16);
		bG2 = (byte)((ulPixel >> 8) & 0xFF);
		bB2 = (byte)(ulPixel & 0xFF);

		nRes = bR2 + bR;
		if ( nRes > 255 ) { bR = 255; }	else { bR = (byte)( nRes ); }
		nRes = bG2 + bG;
		if ( nRes > 255 ) { bG = 255; }	else { bG = (byte)( nRes ); }
		nRes = bB2 + bB;
		if ( nRes > 255 ) { bB = 255; }	else { bB = (byte)( nRes ); }

		*(pulWrite++) = 0xFF000000 | (bR <<16) | (bG<<8) | (bB);

		nPixLoop++;
	} while( nPixLoop < nWidth );
}

/***************************************************************************
 * Function    : CopyLine565Additive
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
inline void CopyLine565Additive ( ushort* puwWrite, byte* pbRead, int nWidth )
{
int	nPixLoop;
ushort	uwPixel;
byte		bR;
byte		bG;
byte		bB;
byte		bR2;
byte		bG2;
byte		bB2;
int		nRes;

	nPixLoop = 0;
	do
	{
		bR = *(pbRead++);
		bG = *(pbRead++);
		bB = *(pbRead++);
		uwPixel = *(puwWrite);
		bR2 = ((uwPixel >> 11)<<3);
		bG2 = (((uwPixel >> 5) & 0x3F)<<2);
		bB2 = ((uwPixel & 0x1F )<<3);

		nRes = bR2 + (bR>>1);
		if ( nRes > 255 ) { bR = 255; }	else { bR = (byte)( nRes ); }
		nRes = bG2 + (bG>>1);
		if ( nRes > 255 ) { bG = 255; }	else { bG = (byte)( nRes ); }
		nRes = bB2 + (bB>>1);
		if ( nRes > 255 ) { bB = 255; }	else { bB = (byte)( nRes ); }

		uwPixel = ((bR >> 3)<< 11) | ((bG>>2)<<5) | (bB>>3);
		*(puwWrite++) = uwPixel ;

		nPixLoop++;
	} while( nPixLoop < nWidth );
}


/***************************************************************************
 * Function    : MyCopyRects
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void MyCopyRects ( COPY_RECT_INFO* pxInfo )
{
BYTE*			pbCurrentLine;
byte*			pbRead;
byte*			pbDestination;
ushort*			puwWrite;
ulong*			pulWrite;
byte		bR;
byte		bG;
byte		bB;
int		nLoop;
int		nPixLoop;

	if ( ( pxInfo->pbDest != NULL ) &&
		 ( pxInfo->pbSource != NULL ) )
	{
		
		nLoop = 0;
		pbCurrentLine = pxInfo->pbSource;
		pbDestination = pxInfo->pbDest;

		while ( nLoop < pxInfo->nHeight )
		{
			pbRead = pbCurrentLine;

			switch ( pxInfo->nDestFormat )
			{
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
				switch ( pxInfo->nFlags )
				{
				case RENDER_TYPE_ADDITIVE:
					CopyLine888Additive( (ulong*)( pbDestination ), pbCurrentLine, pxInfo->nWidth );
					break;
				default:
					pulWrite = (ulong*)( pbDestination );
					nPixLoop = 0;
					do
					{
						bR = *(pbRead++);
						bG = *(pbRead++);
						bB = *(pbRead++);
						*(pulWrite++) = 0xFF000000 | (bR <<16) | (bG<<8) | (bB);
						nPixLoop++;
					} while( nPixLoop < pxInfo->nWidth );
					break;
				}
				break;		
			case D3DFMT_R8G8B8:
				memcpy( pbDestination, pbCurrentLine, (pxInfo->nWidth * 3) );
				break;
			case D3DFMT_X1R5G5B5:
			case D3DFMT_A1R5G5B5:
				switch ( pxInfo->nFlags )
				{
				case RENDER_TYPE_ADDITIVE:
					CopyLine555Additive( (ushort*)( pbDestination ), pbCurrentLine, pxInfo->nWidth );
					break;
				default:
					puwWrite = (ushort*)( pbDestination );
					nPixLoop = 0;
					do
					{
						bR = *(pbRead++);
						bG = *(pbRead++);
						bB = *(pbRead++);
						*(puwWrite++) = ((bR >> 3)<< 10) | ((bG>>3)<<5) | (bB>>3);
						nPixLoop++;
					} while( nPixLoop < pxInfo->nWidth );
					break;
				}
				break;		
			case D3DFMT_R5G6B5:
				switch ( pxInfo->nFlags )
				{
				case RENDER_TYPE_ADDITIVE:
					CopyLine565Additive( (ushort*)( pbDestination ), pbCurrentLine, pxInfo->nWidth );
					break;
				default:
					puwWrite = (ushort*)( pbDestination );
					nPixLoop = 0;
					do
					{
						bR = *(pbRead++);
						bG = *(pbRead++);
						bB = *(pbRead++);
						*(puwWrite++) = ((bR >> 3)<< 11) | ((bG>>2)<<5) | (bB>>3);
						nPixLoop++;
					} while( nPixLoop < pxInfo->nWidth );
					break;
				}
				break;
			default:
				PANIC_IF(TRUE, "Display format not supported for  jpegs. Please ask Mit to support it :]" );
				break;
			}

			pbDestination += pxInfo->nDestPitch;
			pbCurrentLine += pxInfo->nSrcPitch;
			nLoop++;
		}		
	}

}


/***************************************************************************
 * Function    : CopyJpegToBackBuffer
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void CopySurfaces ( IGRAPHICSSURFACE* pxSource,  IGRAPHICSSURFACE* pxDest, POINT* pxDestPoint, int nDestFormat, int nFlags )
{
COPY_RECT_INFO	xCopyRectInfo;
D3DLOCKED_RECT	xLockedRectSrc;
D3DLOCKED_RECT	xLockedRectDest;
RECT			xDestRect;	
	
	pxSource->LockRect( &xLockedRectSrc, &mxJpegRect, 0 );

	if ( xLockedRectSrc.pBits != NULL )
	{
		xDestRect.left = pxDestPoint->x;
		xDestRect.right = pxDestPoint->x + mxJpegRect.right;

		xDestRect.top = pxDestPoint->y;
		xDestRect.bottom = pxDestPoint->y + mxJpegRect.bottom;

		pxDest->LockRect( &xLockedRectDest, NULL, 0 );//&xDestRect, 0 );

		if ( xLockedRectDest.pBits != NULL )
		{
			xCopyRectInfo.nDestPitch = xLockedRectDest.Pitch;
			xCopyRectInfo.nSrcPitch = xLockedRectSrc.Pitch;
			xCopyRectInfo.nDestFormat = nDestFormat;
			xCopyRectInfo.nHeight = mxJpegRect.bottom;
			xCopyRectInfo.nWidth = mxJpegRect.right;
			xCopyRectInfo.nX = 0;
			xCopyRectInfo.nY = 0;
			xCopyRectInfo.nFlags = nFlags;
			xCopyRectInfo.pbDest = (BYTE*)( xLockedRectDest.pBits );
			xCopyRectInfo.pbSource = (BYTE*)( xLockedRectSrc.pBits );

			MyCopyRects( &xCopyRectInfo );

			pxDest->UnlockRect();
		}
		pxSource->UnlockRect();

	}

}


/***************************************************************************
 * Function    : DrawJpegDirect
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void DrawJpegDirect( int nLayer, IGRAPHICSSURFACE* pxTexture, int nX, int nY, int nWidth, int nHeight, int nFlags )
{
	maxBufferedJpegs[ mnNextBufferedJpeg ].nLayer = nLayer;
	maxBufferedJpegs[ mnNextBufferedJpeg ].pxTexture = pxTexture;
	maxBufferedJpegs[ mnNextBufferedJpeg ].nX = nX;
	maxBufferedJpegs[ mnNextBufferedJpeg ].nY = nY;
	maxBufferedJpegs[ mnNextBufferedJpeg ].nWidth = nWidth;
	maxBufferedJpegs[ mnNextBufferedJpeg ].nHeight = nHeight;
	maxBufferedJpegs[ mnNextBufferedJpeg ].nFlags = nFlags;
	mnNextBufferedJpeg++;
	mnNextBufferedJpeg %= MAX_BUFFERED_JPEGS; 

}

void DrawBufferedJpegNew( IGRAPHICSSURFACE* pxTexture, int nX, int nY, int nWidth, int nHeight, int nFlags )
{
IGRAPHICSSURFACE*		pxBackBuffer = NULL;
IGRAPHICSSURFACE*		pxDepthBuffer = NULL;
RECT	xRect; 
RECT	xDestRect;
POINT	xPoint;
HRESULT		hr;
RECT	xSrcRect;
D3DSURFACE_DESC		xSrcDesc;
D3DSURFACE_DESC		xDestDesc;
IGRAPHICSSURFACE*		pxSurface = NULL;

	pxSurface = pxTexture;
	if ( pxSurface != NULL ) 
	{
#ifdef TUD9
		mpInterfaceD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pxBackBuffer );
#else
		mpInterfaceD3DDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pxBackBuffer );
#endif
		if ( pxBackBuffer == NULL )
		{
			PANIC_IF( TRUE, "Couldnt get back buffer" );
			return;
		}

		pxBackBuffer->GetDesc( &xDestDesc );

		switch ( nFlags )
		{
		case RENDER_TYPE_ALPHATEST:
			xPoint.x = nX + mnInterfaceDrawX;
			xPoint.y = nY + mnInterfaceDrawY;
			mpInterfaceD3DDevice->GetDepthStencilSurface( &pxDepthBuffer );
				xRect.left = 0;
				xRect.top = 0;
				xRect.right = nWidth - 1;
				xRect.bottom = nHeight - 1;
				/** And finally copy the merged results back to the back buffer **/
//				hr = mpInterfaceD3DDevice->CopyRects( pxSurface, &xRect, 1, pxDepthBuffer, &xPoint );
			hr = D3DXLoadSurfaceFromSurface( pxDepthBuffer, NULL, NULL, pxSurface, NULL, NULL, 0, 0 );
			pxDepthBuffer->Release();
			PANIC_IF( hr != D3D_OK, "Depth write failed" );
			break;
		case RENDER_TYPE_NORMAL:
		default:
			xPoint.x = nX + mnInterfaceDrawX;
			xPoint.y = nY + mnInterfaceDrawY;
 
			pxSurface->GetDesc( &xSrcDesc );
			xSrcRect.left = 0;
			xSrcRect.top = 0;
//			xSrcRect.right = xSrcDesc.Width - 1;
//			xSrcRect.bottom = xSrcDesc.Height - 1;
			xSrcRect.right = nWidth - 1;
			xSrcRect.bottom = nHeight - 1;
 
			if ( nWidth == 0 ) nWidth = xSrcDesc.Width;
			if ( nHeight == 0 ) nHeight = xSrcDesc.Height;

			if ( 0 ) 
//				 ( xSrcDesc.Width != (ulong)(nWidth) ) ||
//				 ( xSrcDesc.Height != (ulong)(nHeight) ) )
			{
				xRect.left = xPoint.x;
				xRect.top = xPoint.y;
				xRect.right = xRect.left + nWidth - 1;
				xRect.bottom = xRect.top + nHeight - 1;
				xDestRect.left = xPoint.x;
				xDestRect.top = xPoint.y;
				xDestRect.right = xPoint.x + nWidth - 1;
				xDestRect.bottom = xPoint.y + nHeight - 1;
				hr = D3DXLoadSurfaceFromSurface( pxBackBuffer, NULL, &xRect, pxSurface, NULL, &xSrcRect, 0, 0 );
				if ( hr != D3D_OK )
				{
					PANIC_IF( hr != D3D_OK, "Copy rects failed" );
				}
//				pxBackBuffer->AddDirtyRect( &xDestRect );
			}
			else
			{
				xRect.left = 0;
				xRect.top = 0;
				xRect.right = xSrcDesc.Width - 1;
				xRect.bottom = xSrcDesc.Height - 1;
 
				if ( xRect.right >= InterfaceGetWidth() )
				{
					xRect.right = InterfaceGetWidth() - 1;
				}
				if ( xRect.bottom >= InterfaceGetHeight() )
				{
					xRect.bottom = InterfaceGetHeight() - 1;
				}
				/** And finally copy the merged results back to the back buffer **/
#ifdef TUD9
				xDestRect.left = xPoint.x;
				xDestRect.top = xPoint.y;
				xDestRect.right = xPoint.x + nWidth - 1;
				xDestRect.bottom = xPoint.y + nHeight - 1;

				hr = mpInterfaceD3DDevice->StretchRect( pxSurface, &xRect, pxBackBuffer, &xDestRect, D3DTEXF_NONE );
				if ( hr != D3D_OK )
				{
					hr = mpInterfaceD3DDevice->UpdateSurface( pxSurface, &xRect, pxBackBuffer, &xPoint );
					if ( hr != D3D_OK )
					{
						PANIC_IF( hr != D3D_OK, "Surface copy failed" );
					}
				}
#else
				hr = mpInterfaceD3DDevice->CopyRects( pxSurface, &xRect, 1, pxBackBuffer, &xPoint );
#endif
			}
//			PANIC_IF( hr != D3D_OK, "Copy rects failed" );
			break;
		}

		pxBackBuffer->Release();
	}
}


/***************************************************************************
 * Function    : DrawBufferedJpeg
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void DrawBufferedJpeg( IGRAPHICSSURFACE* pxSurface, int nX, int nY, int nWidth, int nHeight, int nFlags )
{
IGRAPHICSSURFACE*		pxBackBuffer = NULL;
IGRAPHICSSURFACE*		pxDepthBuffer = NULL;
RECT	xRect; 
RECT	xDestRect;
POINT	xPoint;
HRESULT		hr;
RECT	xSrcRect;
D3DSURFACE_DESC		xSrcDesc;
D3DSURFACE_DESC		xDestDesc;

	if ( pxSurface != NULL ) 
	{
#ifdef TUD9
		mpInterfaceD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pxBackBuffer );
#else
		mpInterfaceD3DDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pxBackBuffer );
#endif
	
		if ( pxBackBuffer == NULL )
		{
			PANIC_IF( TRUE, "Couldnt get back buffer" );
			return;
		}

		pxBackBuffer->GetDesc( &xDestDesc );

		switch ( nFlags )
		{
		case RENDER_TYPE_ALPHATEST:
			xPoint.x = nX + mnInterfaceDrawX;
			xPoint.y = nY + mnInterfaceDrawY;
			mpInterfaceD3DDevice->GetDepthStencilSurface( &pxDepthBuffer );
				xRect.left = 0;
				xRect.top = 0;
				xRect.right = nWidth - 1;
				xRect.bottom = nHeight - 1;
				/** And finally copy the merged results back to the back buffer **/
//				hr = mpInterfaceD3DDevice->CopyRects( pxSurface, &xRect, 1, pxDepthBuffer, &xPoint );
			hr = D3DXLoadSurfaceFromSurface( pxDepthBuffer, NULL, NULL, pxSurface, NULL, NULL, 0, 0 );
			pxDepthBuffer->Release();
			PANIC_IF( hr != D3D_OK, "Depth write failed" );
			break;
		case RENDER_TYPE_NORMAL:
		default:
			xPoint.x = nX + mnInterfaceDrawX;
			xPoint.y = nY + mnInterfaceDrawY;

			pxSurface->GetDesc( &xSrcDesc );
			xSrcRect.left = 0;
			xSrcRect.top = 0;
			xSrcRect.right = xSrcDesc.Width - 1;
			xSrcRect.bottom = xSrcDesc.Height - 1;

			if ( nWidth == 0 ) nWidth = xSrcDesc.Width;
			if ( nHeight == 0 ) nHeight = xSrcDesc.Height;

			if ( ( xSrcDesc.Width != (ulong)(nWidth) ) ||
				 ( xSrcDesc.Height != (ulong)(nHeight) ) )
			{
				xRect.left = xPoint.x;
				xRect.top = xPoint.y;
				xRect.right = xRect.left + nWidth - 1;
				xRect.bottom = xRect.top + nHeight - 1;
				xDestRect.left = xPoint.x;
				xDestRect.top = xPoint.y;
				xDestRect.right = xPoint.x + nWidth - 1;
				xDestRect.bottom = xPoint.y + nHeight - 1;
				hr = D3DXLoadSurfaceFromSurface( pxBackBuffer, NULL, &xRect, pxSurface, NULL, &xSrcRect, 0, 0 );
//				pxBackBuffer->AddDirtyRect( &xDestRect );
			}
			else
			{
				xRect.left = 0;
				xRect.top = 0;
				xRect.right = nWidth - 1;
				xRect.bottom = nHeight - 1;
 
				if ( xRect.right >= InterfaceGetWidth() )
				{
					xRect.right = InterfaceGetWidth() - 1;
				}
				if ( xRect.bottom >= InterfaceGetHeight() )
				{
					xRect.bottom = InterfaceGetHeight() - 1;
				}
				/** And finally copy the merged results back to the back buffer **/
#ifdef TUD9
				hr = mpInterfaceD3DDevice->UpdateSurface( pxSurface, &xRect, pxBackBuffer, &xPoint );
				if ( hr != D3D_OK )
				{
					xDestRect.left = xPoint.x;
					xDestRect.top = xPoint.y;
					xDestRect.right = xPoint.x + nWidth - 1;
					xDestRect.bottom = xPoint.y + nHeight - 1;
					hr = mpInterfaceD3DDevice->StretchRect( pxSurface, &xRect, pxBackBuffer, &xDestRect, D3DTEXF_POINT );
					if ( hr != D3D_OK )
					{
						PANIC_IF( hr != D3D_OK, "Copy rects failed" );
					}
				}
#else
				hr = mpInterfaceD3DDevice->CopyRects( pxSurface, &xRect, 1, pxBackBuffer, &xPoint );
#endif
			}
//			PANIC_IF( hr != D3D_OK, "Copy rects failed" );
			break;
		}

		pxBackBuffer->Release();
	}

}

void	ClearBufferedJpegs( void )
{	
	mnNextBufferedJpeg = 0;
}

void	DrawBufferedJpegs( int layer )
{	
int		nLoop;

	for ( nLoop = 0; nLoop < mnNextBufferedJpeg; nLoop++ )
	{
		if ( maxBufferedJpegs[nLoop].nLayer == layer )
		{
			DrawBufferedJpegNew( maxBufferedJpegs[nLoop].pxTexture, maxBufferedJpegs[nLoop].nX, maxBufferedJpegs[nLoop].nY, maxBufferedJpegs[nLoop].nWidth, maxBufferedJpegs[nLoop].nHeight, maxBufferedJpegs[nLoop].nFlags );	
		}
	}

	if ( layer == 2 )
	{
		mnNextBufferedJpeg = 0;
	}

}


/***************************************************************************
 * Function    : DrawPrimaryJpeg
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void DrawPrimaryJpeg ( void )
{
	if ( ( mpxJpegSurface != NULL ) &&
		 ( mxJpegDraw.boDisplayThisFrame == TRUE ) )
	{
	IGRAPHICSSURFACE*		pxBackBuffer = NULL;
	D3DDISPLAYMODE	xDisplayMode;
	RECT	xRect;
	POINT	xCopyDestPoint;

#ifdef TUD9
		mpInterfaceD3DDevice->GetDisplayMode( 0, &xDisplayMode );
		mpInterfaceD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pxBackBuffer );
#else
		mpInterfaceD3DDevice->GetDisplayMode( &xDisplayMode );
		mpInterfaceD3DDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pxBackBuffer );
#endif
		if ( pxBackBuffer == NULL )
		{
			PANIC_IF( TRUE, "Couldnt get back buffer" );
			return;
		}

		switch ( mxJpegDraw.nFlags )
		{
		case RENDER_TYPE_ADDITIVE:
			// Copy the back buffer to the spare surface 
			xCopyDestPoint.x = 0;
			xCopyDestPoint.y = 0;
			xRect.left = mxJpegDraw.xPoint.x;
			xRect.top = mxJpegDraw.xPoint.y;
			xRect.right = xRect.left + mxJpegRect.right;
			xRect.bottom = xRect.top + mxJpegRect.bottom;

#ifdef TUD9
			mpInterfaceD3DDevice->UpdateSurface( pxBackBuffer, &xRect, mpxBackBufferCopySurface, &xCopyDestPoint );
#else
			mpInterfaceD3DDevice->CopyRects( pxBackBuffer, &xRect, 1, mpxBackBufferCopySurface, &xCopyDestPoint );
#endif
			// Now copy (and merge) the jpeg with the copy from the back buffer 
			CopySurfaces( mpxJpegSurface,mpxBackBufferCopySurface, &xCopyDestPoint, xDisplayMode.Format, mxJpegDraw.nFlags );

			// And finally copy the merged results back to the back buffer 
#ifdef TUD9
			mpInterfaceD3DDevice->UpdateSurface( mpxBackBufferCopySurface, &mxJpegRect, pxBackBuffer, &mxJpegDraw.xPoint );
#else
			mpInterfaceD3DDevice->CopyRects( mpxBackBufferCopySurface, &mxJpegRect, 1, pxBackBuffer, &mxJpegDraw.xPoint );
#endif
			break;
		case RENDER_TYPE_NORMAL:
		default:
			/// And finally copy the merged results back to the back buffer 
#ifdef TUD9
			mpInterfaceD3DDevice->UpdateSurface( mpxBackBufferCopySurface, &mxJpegRect, pxBackBuffer, &mxJpegDraw.xPoint );
#else
			mpInterfaceD3DDevice->CopyRects( mpxBackBufferCopySurface, &mxJpegRect, 1, pxBackBuffer, &mxJpegDraw.xPoint );
#endif
			break;
		}

	
		pxBackBuffer->Release();
		
		mxJpegDraw.boDisplayThisFrame = FALSE;
	}

}


/***************************************************************************
 * Function    : JpegGetDrawPoint
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void JpegGetDrawPoint( int* pnX, int *pnY )
{
	*( pnX ) = mxJpegDraw.xPoint.x;
	*( pnY ) = mxJpegDraw.xPoint.y;

}

/***************************************************************************
 * Function    : ShowJpeg
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void ShowJpeg ( int nX, int nY, int nWidth, int nHeight, int nFlags )
{

	mxJpegDraw.xPoint.x = nX + mnInterfaceDrawX;
	mxJpegDraw.xPoint.y = nY + mnInterfaceDrawY;

	mxJpegDraw.boDisplayThisFrame = TRUE;

	mxJpegDraw.nFlags = nFlags;

}


/***************************************************************************
 * Function    : ShowJpegCentered
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void ShowJpegCentered( int nX, int nY, int nWidth, int nHeight, int nFlags )
{
int	nXGap;
int	nYGap;

	nXGap = (nWidth - mxJpegRect.right ) / 2;
	nYGap = (nHeight - mxJpegRect.bottom ) / 2;

	ShowJpeg( nX + nXGap, nY + nYGap, 0, 0, nFlags );


}



/***************************************************************************
 * Function    : FreeJpeg
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void FreeJpeg ( void )
{
	if ( mpxJpegSurface != NULL )
	{
		mpxJpegSurface->Release();
		mpxJpegSurface = NULL;
	}

	if ( mpxBackBufferCopySurface != NULL )
	{
		mpxBackBufferCopySurface->Release();
		mpxBackBufferCopySurface = NULL;
	}

}

//#define USE_DXTEXTURES_FOR_JPEG_LOADER

#ifdef USE_DXTEXTURES_FOR_JPEG_LOADER
LPGRAPHICSTEXTURE LoadJpegDirect( const char* szFilename, int nWidth, int nHeight, int nFlags )
{
D3DDISPLAYMODE	xDisplayMode;
LPGRAPHICSTEXTURE	pTexture = NULL;
int		nRet;

	mpInterfaceD3DDevice->GetDisplayMode( 0, &xDisplayMode );
//	

	nRet = D3DXCreateTextureFromFileEx( mpInterfaceD3DDevice, szFilename, nWidth, nHeight, 1, 0, xDisplayMode.Format, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &pTexture );
//	nRet = D3DXCreateTextureFromFileEx( mpInterfaceD3DDevice, szFilename, nWidth, nHeight, 1, 0, xDisplayMode.Format, D3DPOOL_SYSTEMMEM, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &pTexture );
	if( FAILED( nRet ) )
	{
		InterfaceTextureLoadError( nRet,szFilename );
	}
	return( pTexture );
}

#else
/***************************************************************************
 * Function    : LoadJpegDirect
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
IGRAPHICSSURFACE* LoadJpegDirect( const char* szFilename, int nWidth, int nHeight, int nFlags )
{
IGRAPHICSSURFACE*		pDestSurface = NULL;
int		nRet;
D3DSURFACE_DESC		xDestDesc;
IGRAPHICSSURFACE*		pxBackBuffer = NULL;

	if ( mpInterfaceD3DDevice == NULL ) return( NULL );

	if ( nWidth == 0 )
	{
	D3DXIMAGE_INFO Info; 

		D3DXGetImageInfoFromFile( szFilename, &Info);
		nWidth = Info.Width;
		nHeight = Info.Height;
	}

	mpInterfaceD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pxBackBuffer );
	if ( pxBackBuffer == NULL )
	{
		PANIC_IF( TRUE, "Couldnt get back buffer" );
		return ( NULL );
	}
	pxBackBuffer->GetDesc( &xDestDesc );

	mpInterfaceD3DDevice->CreateOffscreenPlainSurface( nWidth, nHeight, xDestDesc.Format, D3DPOOL_DEFAULT, &pDestSurface, NULL); 

	if ( pDestSurface )
	{
		nRet = D3DXLoadSurfaceFromFile(pDestSurface, NULL, NULL, (LPCSTR)szFilename, NULL, D3DX_DEFAULT, 0, NULL );
	}

	// Release coz 'GetBackBuffer' increases reference count
	pxBackBuffer->Release();

	return( pDestSurface );
}

/***************************************************************************
 * Function    : LoadJpegDirect
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
IGRAPHICSSURFACE* LoadJpegDirectArchive( const char* szFilename, int nWidth, int nHeight, int nFlags, int nArchiveHandle )
{
IGRAPHICSSURFACE*		pDestSurface = NULL;
int		nRet;
D3DSURFACE_DESC		xDestDesc;
IGRAPHICSSURFACE*		pxBackBuffer = NULL;
Archive*		pArchive = ArchiveGetFromHandle( nArchiveHandle );
byte*	pbFileInMem;
int		nFileHandle;
int		nFileSize;

	if ( mpInterfaceD3DDevice == NULL ) return( NULL );

	nFileHandle = pArchive->OpenFile( szFilename );

	if ( nFileHandle > 0 )
	{
		// Get the size of the tex
		nFileSize = pArchive->GetFileSize( nFileHandle );

		pbFileInMem = (byte*)( SystemMalloc( nFileSize ) );
		if ( pbFileInMem != NULL )
		{
			pArchive->ReadFile( nFileHandle, pbFileInMem, nFileSize );
			pArchive->CloseFile( nFileHandle );

			if ( nWidth == 0 )
			{
			D3DXIMAGE_INFO Info; 

				D3DXGetImageInfoFromFileInMemory( pbFileInMem, nFileSize, &Info);
				nWidth = Info.Width;
				nHeight = Info.Height;
			}

			mpInterfaceD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pxBackBuffer );
			if ( pxBackBuffer == NULL )
			{
				free( pbFileInMem );
				PANIC_IF( TRUE, "Couldnt get back buffer" );
				return ( NULL );
			}
			pxBackBuffer->GetDesc( &xDestDesc );
			pxBackBuffer->Release();
			mpInterfaceD3DDevice->CreateOffscreenPlainSurface( nWidth, nHeight, xDestDesc.Format, D3DPOOL_DEFAULT, &pDestSurface, NULL); 

			if ( pDestSurface )
			{
				nRet = D3DXLoadSurfaceFromFileInMemory(pDestSurface, NULL, NULL, (LPVOID)pbFileInMem, nFileSize, NULL, D3DX_DEFAULT, D3DCOLOR(0xFF), NULL );
			}
			free( pbFileInMem );
		}
	}
	return( pDestSurface );
}
#endif

/***************************************************************************
 * Function    : LoadJpeg
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void LoadJpeg ( const char* szFilename )
{
D3DDISPLAYMODE	xDisplayMode;
JpegFile		xJpegFile;
D3DLOCKED_RECT	xLockedRect;
BYTE*			pbRGBBuffer = NULL;
BYTE*			pbCurrentLine;
byte*			pbDestination;
#ifndef INTERFACE_API
char	acString[256];
#endif
uint	ulWidth;
uint	ulHeight;
int		nLoop;

	mxJpegDraw.boDisplayThisFrame = FALSE;

	if ( InterfaceIsSmall() || mpInterfaceD3DDevice == NULL )
	{
		return;
	}

	if ( szFilename == NULL )
	{
		if ( mszLastJpegFile[0] != 0 )
		{
			szFilename = mszLastJpegFile;
		}
		else
		{
			return;
		}
	}
	pbRGBBuffer = JpegFile::JpegFileToRGB(szFilename, &ulWidth, &ulHeight );

	if ( pbRGBBuffer == NULL )
	{
#ifndef INTERFACE_API
		sprintf (acString,"Unable to load jpeg file %s", szFilename );
		PrintConsoleCR( acString, COL_WARNING );
#endif
		return;
	}

	if ( ( ulWidth > 800 ) || ( ulHeight > 800 ) )
	{
#ifndef INTERFACE_API
		sprintf (acString,"Jpeg file %s is too large. (Max 800x800)", szFilename );
		PrintConsoleCR( acString, COL_WARNING );
#endif
		delete [] pbRGBBuffer;
		return;
	}

	if ( FAILED( InterfaceInternalDXCreateImageSurface( ulWidth, ulHeight, FORMAT_R8G8B8, &mpxJpegSurface ) ) )
	{
#ifndef INTERFACE_API
		PrintConsoleCR( "Couldnt create jpeg image surface", COL_WARNING );
#endif
		delete [] pbRGBBuffer;
		return;
	}

	if ( FAILED( InterfaceInternalDXCreateImageSurface( ulWidth, ulHeight, FORMAT_DISPLAY, &mpxBackBufferCopySurface ) ) )
	{
#ifndef INTERFACE_API
		PrintConsoleCR( "Couldnt create jpeg backbuffer copy surface", COL_WARNING );
#endif
		return;
	}


	if ( szFilename != mszLastJpegFile )
	{
		sprintf( mszLastJpegFile, szFilename );
	}

	/** Lock the surface we've created **/
	mxJpegRect.top = 0;
	mxJpegRect.left = 0;
	mxJpegRect.bottom = ulHeight;
	mxJpegRect.right = ulWidth;

	mpxJpegSurface->LockRect( &xLockedRect, &mxJpegRect, 0 );

	if ( xLockedRect.pBits != NULL )
	{
		nLoop = 0;
		pbCurrentLine = pbRGBBuffer;
		pbDestination = (byte*)( xLockedRect.pBits );

		while ( nLoop < (int)( ulHeight ) )
		{
			memcpy( pbDestination, pbCurrentLine, (ulWidth * 3) );

			pbDestination += xLockedRect.Pitch;
			pbCurrentLine += (ulWidth*3);
			nLoop++;
		}		
	}
	mpxJpegSurface->UnlockRect();

	mxJpegDraw.xPoint.x = 0;
	mxJpegDraw.xPoint.y = 0;

	mpInterfaceD3DDevice->GetDisplayMode( 0, &xDisplayMode );

	/** Copy it into the spare buffer (i.e change its format) **/
	CopySurfaces( mpxJpegSurface, mpxBackBufferCopySurface, &mxJpegDraw.xPoint, xDisplayMode.Format, RENDER_TYPE_NORMAL );

	delete [] pbRGBBuffer;

}

#else		// ! TUD9




#endif
