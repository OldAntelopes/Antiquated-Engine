
#ifdef USING_OPENGL
	
#include <windows.h>				// For OpenGL
#include "gl/glew.h"
#include "gl/gl.h"
#include "../InterfaceInternalsGL.h"

#include <StandardDef.h>
#include <Interface.h>

#include "../../Common/InterfaceUtil.h"
#include "../../Common/Overlays/Overlays.h"

 
//------------------------------------------------------------------------------------------------------------

#define		LINE_VERTEX_BUFFER_SIZE			65536


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
}


/***************************************************************************
 * Function    : InterfaceRect
 * Params      : 
 * Returns     :
 * Description : OpenGL version
 ***************************************************************************/
INTERFACE_API void InterfaceRect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol)
{
	if ( nLayer == 0 )
	{
		InterfaceShadedRect( 0, nX, nY, nWidth, nHeight, ulCol, ulCol, ulCol, ulCol );
	}
	else
	{
		InterfaceShadedRect( 1, nX, nY, nWidth, nHeight, ulCol, ulCol, ulCol, ulCol );
	}
}

/***************************************************************************
 * Function    : AddBox
 * Params      : 
 * Returns     :
 * Description : OpenGL version
 ***************************************************************************/
INTERFACE_API void InterfaceOutlineBox ( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol )
{
	InterfaceRect( 0, nX - 2, nY - 2, 2, nHeight + 4, ulCol );
	InterfaceRect( 0, nX + nWidth, nY - 2, 2, nHeight + 4, ulCol );

	InterfaceRect( 0, nX, nY - 2, nWidth, 2, ulCol );
	InterfaceRect( 0, nX, nY + nHeight, nWidth, 2, ulCol );

}
/***************************************************************************
 * Function    : InterfaceShadedBox
 * Params      : 
 * Returns     :
 * Description : OpenGL version
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
			InterfaceRect( 1, nX - 2, nY - 2, 2, nHeight + 4, 0x404080CF );
			InterfaceRect( 1, nX + nWidth, nY - 2, 2, nHeight + 4, 0x404080CF );

			InterfaceRect( 1, nX, nY - 2, nWidth, 2, 0x404080CF );
			InterfaceRect( 1, nX, nY + nHeight, nWidth, 2, 0x404080CF );
			break;
		case 1:
			InterfaceRect( 1, nX, nY, nWidth, nHeight, 0x70000000 );
			InterfaceRect( 1, nX - 2, nY - 2, 2, nHeight + 4, 0x404080CF );
			InterfaceRect( 1, nX + nWidth, nY - 2, 2, nHeight + 4, 0x404080CF );

			InterfaceRect( 1, nX, nY - 2, nWidth, 2, 0x404080CF );
			InterfaceRect( 1, nX, nY + nHeight, nWidth, 2, 0x404080CF );
			break;
		case 2:
			InterfaceRect( 1, nX, nY, nWidth, nHeight, 0x70404040 );
			InterfaceRect( 1, nX - 2, nY - 2, 2, nHeight + 4, 0x60B0B0B0 );
			InterfaceRect( 1, nX + nWidth, nY - 2, 2, nHeight + 4, 0x60B0B0B0 );

			InterfaceRect( 1, nX, nY - 2, nWidth, 2, 0x60101010 );
			InterfaceRect( 1, nX, nY + nHeight, nWidth, 2, 0x60101010 );
			break;
		}
	}
	else 
	{
		InterfaceRect( 1, nX, nY, nWidth, nHeight, (uint32)(nStyle) );

		InterfaceRect( 1, nX - 2, nY - 2, 2, nHeight + 4, 0x404080CF );
		InterfaceRect( 1, nX + nWidth, nY - 2, 2, nHeight + 4, 0x404080CF );

		InterfaceRect( 1, nX, nY - 2, nWidth, 2, 0x404080CF );
		InterfaceRect( 1, nX, nY + nHeight, nWidth, 2, 0x404080CF );
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
 * Description : OpenGL version
 ***************************************************************************/
void RenderOverlays( int nLayer )
{
}


/***************************************************************************
 * Function    : InitialiseOverlays
 * Params      :
 * Returns     :
 * Description : OpenGL version
 ***************************************************************************/
HRESULT InitialiseOverlays( void )
{

	return S_OK;

}


/***************************************************************************
 * Function    : FreeOverlays
 * Params      :
 * Returns     :
 * Description : OpenGL version
 ***************************************************************************/
void FreeOverlays( void )
{
} 


/***************************************************************************
 * Function    : LockOverlays
 * Params      :
 * Returns     :
 * Description : OpenGL version
 ***************************************************************************/
void LockOverlays( void )
{

}



/***************************************************************************
 * Function    : UnlockOverlays
 * Params      :
 * Returns     :
 * Description : OpenGL version
 ***************************************************************************/
void UnlockOverlays( void )
{
}

INTERFACE_API void	InterfaceTri( int nLayer, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, uint32 ulCol1, uint32 ulCol2, uint32 ulCol3 )
{
float	fR, fG, fB, fA;

	// todo
	// TEMP - Should be adding to drawlist
    glDisable (GL_TEXTURE_2D); /* disable texture mapping */

	glBegin (GL_TRIANGLES);

	InterfaceUnpackCol( ulCol1, &fR, &fG, &fB, &fA );
	glColor4f( fR, fG, fB, fA );
	glVertex3f( (GLfloat)nX1, (GLfloat)nY1, 0.0f);
	InterfaceUnpackCol( ulCol2, &fR, &fG, &fB, &fA );
	glColor4f( fR, fG, fB, fA );
	glVertex3f ((GLfloat)nX2, (GLfloat)nY2, 0.0f);
	InterfaceUnpackCol( ulCol3, &fR, &fG, &fB, &fA );
	glColor4f( fR, fG, fB, fA );
	glVertex3f ((GLfloat)nX3, (GLfloat)nY3, 0.0f);

	glEnd ();

}

INTERFACE_API void InterfaceLine( int nLayer, int nX1, int nY1, int nX2, int nY2, uint32 ulCol1, uint32 ulCol2 )
{
float	fR, fG, fB, fA;

	// todo
	glDisable (GL_TEXTURE_2D); /* disable texture mapping */

	glBegin (GL_LINES);

	InterfaceUnpackCol( ulCol1, &fR, &fG, &fB, &fA );
	glColor4f( fR, fG, fB, fA );
	glVertex3f ((GLfloat)nX1, (GLfloat)nY1, 0.0f);
	InterfaceUnpackCol( ulCol2, &fR, &fG, &fB, &fA );
	glColor4f( fR, fG, fB, fA );
	glVertex3f ((GLfloat)nX2, (GLfloat)nY2, 0.0f);

	glEnd ();
}



#endif	// #ifdef USING_OPENGL
