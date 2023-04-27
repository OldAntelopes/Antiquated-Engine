#include <stdio.h>
#include <windows.h>
#include <CommCtrl.h>
#include <shlobj.h>
#include "resource.h"

#include <d3dx9.h>


#include <StandardDef.h>
#include <System.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#include "../LibCode/Interface/DirectX/InterfaceInternalsDX.h"
#include "../LibCode/Interface/Common/Overlays/Overlays.h"
#include "../LibCode/Engine/ModelFiles.h"
#include "../LibCode/Engine/ModelRendering.h"
#include "../LibCode/Engine/ModelMaterialData.h"

#include "Scene/ModelEditorSceneObject.h"
#include "ModelConverter.h"
#include "UVUnwrap.h"


#define		NUM_QUAD_VERTICES	16
#define		NUM_LINE_VERTICES	(65536*4)


LPDIRECT3DDEVICE9       mpUVUnwrapD3DDevice = NULL; // Our rendering device
HWND					mhwndUVUnwrapMain = NULL;
LPDIRECT3DVERTEXBUFFER9		mpQuadVertexBuffer = NULL;
LPDIRECT3DVERTEXBUFFER9		mpLineVertexBuffer = NULL;
LPDIRECT3DTEXTURE9			mpMapTexture = NULL;

MAPPING_LIST*	mpMappingList = NULL;
MAPPING_LIST*	mpOriginalMappingList = NULL;
MAPPING_LIST*	mpUndoMappingList = NULL;
MAPPING_LIST*	mpToolStartMappingList = NULL;

int				mnMappingNumFaces = 0;
CSceneObject*	mpSceneObject = NULL;

int		mnUVRenderBufferW = 0;
int		mnUVRenderBufferH = 0;

struct MappingView
{
	float	fTopLeftU;
	float	fTopLeftV;
	float	fBotRightU;
	float	fBotRightV;
};

MappingView		m_MappingView = { -0.1f, -0.1f, 1.1f, 1.1f };  


BOOL	mbUVLeftMouseDown = FALSE;
BOOL	mbUVRightMouseDown = FALSE;
BOOL	mbUVMidMouseDown = FALSE;
VECT	mxUVMousePos;
VECT	mxUVMouseDownPos;

VECT	mxUVSelectionBox1;
VECT	mxUVSelectionBox2;
BOOL	mbSelectionBoxDisplay = FALSE;
BOOL	mbIsMakingSelection = FALSE;
float	mfUVUnwrapViewScale = 1.0f;

enum eUVCONTROL_MODE
{
	ZOOM_BOX,
	MOVEXY,
	MOVEX,
	MOVEY,
	ROTATE,
	SCALEXY,
	SCALEX,
	SCALEY,
	MIRROR,
	FLIP,
};

eUVCONTROL_MODE		m_UVControlMode = ZOOM_BOX;

void	UVUnwrapRender( void );

void		UVUnwrapResetView( void )
{
MappingView		m_DefaultMappingView = { -0.1f, -0.1f, 1.1f, 1.1f };  
	m_MappingView = m_DefaultMappingView;
	UVUnwrapRender();

}

void	UVUnwrapSetControlMode( eUVCONTROL_MODE mode )
{
	mbSelectionBoxDisplay = FALSE;
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK1, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK3, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK4, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK5, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK6, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK7, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK8, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK11, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK12, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK13, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK14, BM_SETCHECK, BST_UNCHECKED, 0 );

	switch( mode )
	{
	case ZOOM_BOX:
		SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK1, BM_SETCHECK, BST_CHECKED, 0 );
		break;
	case SCALEXY:
		SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK11, BM_SETCHECK, BST_CHECKED, 0 );
		break;
	case SCALEX:
		SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK12, BM_SETCHECK, BST_CHECKED, 0 );
		break;
	case SCALEY:
		SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK14, BM_SETCHECK, BST_CHECKED, 0 );
		break;
	case MOVEX:
		SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK3, BM_SETCHECK, BST_CHECKED, 0 );
		break;
	case MOVEY:
		SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK4, BM_SETCHECK, BST_CHECKED, 0 );
		break;
	case MOVEXY:
		SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK5, BM_SETCHECK, BST_CHECKED, 0 );
		break;
	case ROTATE:
		SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK6, BM_SETCHECK, BST_CHECKED, 0 );
		break;
	case MIRROR:
		SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK7, BM_SETCHECK, BST_CHECKED, 0 );
		break;
	case FLIP:
		SendDlgItemMessage( mhwndUVUnwrapMain, IDC_CHECK8, BM_SETCHECK, BST_CHECKED, 0 );
		break;
	}

	m_UVControlMode = mode;
}

void	UVUnwrapRenderMappingGetCoord( float fU, float fV, int* pnX, int* pnY )
{
float	fX;
float	fY;

	fX = fU - m_MappingView.fTopLeftU;
	fX = fX / ( m_MappingView.fBotRightU - m_MappingView.fTopLeftU );
	// (fX is now in range 0.0f to 1.0f)
	fX = fX * ( mnUVRenderBufferW );
	*pnX = (int)( fX );

	fY = fV - m_MappingView.fTopLeftV;
	fY = fY / ( m_MappingView.fBotRightV - m_MappingView.fTopLeftV );
	// (fX is now in range 0.0f to 1.0f)
	fY = fY * ( mnUVRenderBufferH );
	*pnY = (int)( fY );
}


void	UVUnwrapRenderMappingGetUV( int X, int Y, float* pfU, float* pfV )
{
float	fU, fV;

	fU = (float)X;
	fU = fU / ( mnUVRenderBufferW );
	fU = fU * ( m_MappingView.fBotRightU - m_MappingView.fTopLeftU );
	fU = fU + m_MappingView.fTopLeftU;
	*pfU = fU;

	fV = (float)Y;
	fV = fV / ( mnUVRenderBufferH );
	fV = fV * ( m_MappingView.fBotRightV - m_MappingView.fTopLeftV );
	fV = fV + m_MappingView.fTopLeftV;
	*pfV = fV;
}


VECT			UVUnwrapGetMappingMidpoint( void )
{
VECT	xOrigin = { 0.0f, 0.0f, 0.0f };
int		loop;
int		nVertCount = 0;
MAPPING_LIST*		pMapping = &mpMappingList[0];

	for ( loop = 0; loop < mnMappingNumFaces; loop++ )
	{
		if ( pMapping->nFlags & 0x1 )
		{
			xOrigin.x += pMapping->u1;
			xOrigin.y += pMapping->v1;
			nVertCount++;
		}
		if ( pMapping->nFlags & 0x2 )
		{
			xOrigin.x += pMapping->u2;
			xOrigin.y += pMapping->v2;
			nVertCount++;
		}
		if ( pMapping->nFlags & 0x4 )
		{
			xOrigin.x += pMapping->u3;
			xOrigin.y += pMapping->v3;
			nVertCount++;
		}
	}

	if ( nVertCount > 0 )
	{
		xOrigin.x /= nVertCount;
		xOrigin.y /= nVertCount;
	}
	return( xOrigin );
}


void			UVUnwrapScaleAllSelectedVerts( BOOL bModifyU, BOOL bModifyV, float fScaleSpeed )
{
float		fOffsetX = mxUVMousePos.x - mxUVMouseDownPos.x;
float		fOffsetY = mxUVMousePos.y - mxUVMouseDownPos.y;
float	fMoveScale = 0.001f * mfUVUnwrapViewScale * fScaleSpeed;
float		fDiff = 1.0f + ( (fOffsetX + fOffsetY) * fMoveScale);
MAPPING_LIST*		pMapping = mpMappingList;
MAPPING_LIST*		pSourceMapping = mpToolStartMappingList;
VECT		xOrigin = UVUnwrapGetMappingMidpoint();
int		loop;

	if ( fDiff < 0.01f )
	{
		fDiff = 0.01f;
	}
	else if ( fDiff > 4.0f )
	{
		fDiff = 4.0f;
	}

	for ( loop = 0; loop < mnMappingNumFaces; loop++ )
	{
		if ( pMapping->nFlags & 0x1 )
		{
			if ( bModifyU )
			{
				pMapping->u1 = xOrigin.x + ((pSourceMapping->u1 - xOrigin.x) * fDiff);
			}
			if ( bModifyV )
			{
				pMapping->v1 = xOrigin.y + ((pSourceMapping->v1 - xOrigin.y) * fDiff);
			}
		}
		if ( pMapping->nFlags & 0x2 )
		{
			if ( bModifyU )
			{
				pMapping->u2 = xOrigin.x + ((pSourceMapping->u2 - xOrigin.x) * fDiff);
			}
			if ( bModifyV )
			{
				pMapping->v2 = xOrigin.y + ((pSourceMapping->v2 - xOrigin.y) * fDiff);
			}
		}
		if ( pMapping->nFlags & 0x4 )
		{
			if ( bModifyU )
			{
				pMapping->u3 = xOrigin.x + ((pSourceMapping->u3 - xOrigin.x) * fDiff);
			}
			if ( bModifyV )
			{
				pMapping->v3 = xOrigin.y + ((pSourceMapping->v3 - xOrigin.y) * fDiff);
			}
		}
		pMapping++;
		pSourceMapping++;
	}
	mpSceneObject->ApplyMappingListUVs( mpMappingList, mnMappingNumFaces );
}

void	UVUnwrapMoveAllSelectedVerts( float fX, float fY )
{
int		loop;
MAPPING_LIST*		pMapping = &mpMappingList[0];

	for ( loop = 0; loop < mnMappingNumFaces; loop++ )
	{
		if ( pMapping->nFlags & 0x1 )
		{
			pMapping->u1 += fX;
			pMapping->v1 += fY;
		}
		if ( pMapping->nFlags & 0x2 )
		{
			pMapping->u2 += fX;
			pMapping->v2 += fY;
		}
		if ( pMapping->nFlags & 0x4 )
		{
			pMapping->u3 += fX;
			pMapping->v3 += fY;
		}
		pMapping++;
	}
	mpSceneObject->ApplyMappingListUVs( mpMappingList, mnMappingNumFaces );
}



BOOL	UVUnwrapWasMouseOverSelectedVertex( int nMouseX, int nMouseY )
{
int		loop;
MAPPING_LIST*		pMapping = &mpMappingList[0];
int		x,y = 0;
int		nSelectionRangePixels = 3;

	pMapping = &mpMappingList[0];
	for ( loop = 0; loop < mnMappingNumFaces; loop++ )
	{
		// If vertex selected
		if ( pMapping->nFlags & 0x1 )
		{
			UVUnwrapRenderMappingGetCoord( pMapping->u1, pMapping->v1, &x, &y );
			if ( ( nMouseX >= x-nSelectionRangePixels ) &&
				 ( nMouseX <= x+nSelectionRangePixels ) &&
				 ( nMouseY >= y-nSelectionRangePixels ) &&
				 ( nMouseY <= y+nSelectionRangePixels ) )
			{
				return( TRUE );
			}
		}
		if ( pMapping->nFlags & 0x2 )
		{
			UVUnwrapRenderMappingGetCoord( pMapping->u2, pMapping->v2, &x, &y );
			if ( ( nMouseX >= x-nSelectionRangePixels ) &&
				 ( nMouseX <= x+nSelectionRangePixels ) &&
				 ( nMouseY >= y-nSelectionRangePixels ) &&
				 ( nMouseY <= y+nSelectionRangePixels ) )
			{
				return( TRUE );
			}
		}
		if ( pMapping->nFlags & 0x4 )
		{
			UVUnwrapRenderMappingGetCoord( pMapping->u3, pMapping->v3, &x, &y );
			if ( ( nMouseX >= x-nSelectionRangePixels ) &&
				 ( nMouseX <= x+nSelectionRangePixels ) &&
				 ( nMouseY >= y-nSelectionRangePixels ) &&
				 ( nMouseY <= y+nSelectionRangePixels ) )
			{
				return( TRUE );
			}
		}
		pMapping++;
	}
	return( FALSE );
}

void	UVUnwrapOnLeftMouseDown( void )
{
	mbIsMakingSelection = FALSE;

	switch( m_UVControlMode )
	{
	case MOVEX:
	case MOVEY:
	case MOVEXY:
		// TODO - if the mousedown was over any of the selected vertices, we move em
		// otherwise set..
		if ( UVUnwrapWasMouseOverSelectedVertex( (int)mxUVMouseDownPos.x, (int)mxUVMouseDownPos.y ) )
		{
			mbIsMakingSelection = FALSE;
		}
		else
		{
			mbIsMakingSelection = TRUE;
		}
		break;
	case ZOOM_BOX:
		mbIsMakingSelection = TRUE;
		break;
	}

}




void	UVUnwrapLeftMouseDownMove( float fDeltaX, float fDeltaY )
{
float	fMove;
float	fMoveScale = 0.001f * mfUVUnwrapViewScale;

	if ( mbIsMakingSelection )
	{
		mxUVSelectionBox1 = mxUVMouseDownPos;
		mxUVSelectionBox2 = mxUVMousePos;
		mbSelectionBoxDisplay = TRUE;
	}
	else
	{
		switch( m_UVControlMode )
		{
		case MOVEX:
			fMove = (fDeltaX + fDeltaY)	* fMoveScale;
			UVUnwrapMoveAllSelectedVerts( fMove, 0.0f );
			break;
		case MOVEY:
			fMove = (fDeltaX + fDeltaY)	* fMoveScale;
			UVUnwrapMoveAllSelectedVerts( 0.0f, fMove );
			break;
		case MOVEXY:
			fMove = (fMoveScale*2.0f);
			UVUnwrapMoveAllSelectedVerts( (fDeltaX * fMove), (fDeltaY * fMove) );
			break;
		case SCALEXY:
			UVUnwrapScaleAllSelectedVerts( TRUE, TRUE, 2.0f );
			break;
		case SCALEX:
			UVUnwrapScaleAllSelectedVerts( TRUE, FALSE, 1.0f );
			break;
		case SCALEY:
			UVUnwrapScaleAllSelectedVerts( FALSE, TRUE, 1.0f);
			break;
		default:
			break;
		}
	}
}





BOOL UVUnwrapDeviceCreateTestDepth(LPDIRECT3D9	pD3D, D3DFORMAT fmt, D3DDISPLAYMODE d3ddm)
{
    if (D3D_OK!=pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,d3ddm.Format,
                                         D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,fmt))
        return FALSE;
    if (D3D_OK!=pD3D->CheckDepthStencilMatch(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,
                                              d3ddm.Format,d3ddm.Format,fmt))
        return FALSE;
    return TRUE;
}


void	UVUnwrapWindowShutdown( void )
{
	if ( mpSceneObject )
	{
		mpSceneObject->EnableSelectedFaceHighlight( true );
	}
	SAFE_DELETE_ARRAY( mpMappingList );
	SAFE_DELETE_ARRAY( mpOriginalMappingList );
	SAFE_DELETE_ARRAY( mpToolStartMappingList );
	
    if( mpQuadVertexBuffer != NULL )
	{
        mpQuadVertexBuffer->Release();
		mpQuadVertexBuffer = NULL;
	}
    if( mpLineVertexBuffer != NULL )
	{
        mpLineVertexBuffer->Release();
		mpLineVertexBuffer = NULL;
	}
	if( mpMapTexture != NULL )
	{
        mpMapTexture->Release();
		mpMapTexture = NULL;
	}
	mpUVUnwrapD3DDevice->SetTexture(0, NULL);
	mpUVUnwrapD3DDevice->SetTexture(1, NULL);
//	mpUVUnwrapD3DDevice->ResourceManagerDiscardBytes( 0 );

	mpUVUnwrapD3DDevice->Release();
	mpUVUnwrapD3DDevice = NULL;
}


LPDIRECT3DDEVICE9 UVUnwrapWindowCreateDevice( HWND hWindow )
{
LPDIRECT3DDEVICE9		pDevice;
D3DXVECTOR3 xVect;
HRESULT		hr;
D3DDISPLAYMODE d3ddm;
BOOL boMinPageSize = FALSE;
bool		boDidLimitTo900ByX = false;
bool		boDidLimitTo900ByY = false;
bool		bFullScreenAntiAlias = false;
LPDIRECT3D9	pD3D = InterfaceGetD3D();

	// Get the current desktop display mode, so we can set up a back
	// buffer of the same format
    if( FAILED( pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
	{
		PANIC_IF(TRUE,"Couldn't get display mode" );
	    return( NULL );
	}
	
	/** Set up the present parameters - This is generally what odd vid cards have a problem with **/
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
	
    // Set windowed-mode style
	d3dpp.Windowed = TRUE;

	RECT	xRect;
	GetWindowRect( hWindow, &xRect );
	mnUVRenderBufferW = xRect.right - xRect.left;
	mnUVRenderBufferH = xRect.bottom - xRect.top;

	d3dpp.BackBufferWidth  = mnUVRenderBufferW;
	d3dpp.BackBufferHeight = mnUVRenderBufferH;

    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D32;
    d3dpp.hDeviceWindow          = hWindow;

	// Find a decent fmt for the depth buffer
	if ( UVUnwrapDeviceCreateTestDepth(pD3D,D3DFMT_D32,d3ddm) == TRUE )
	{
		d3dpp.AutoDepthStencilFormat = D3DFMT_D32;
	}
	else if ( UVUnwrapDeviceCreateTestDepth(pD3D,D3DFMT_D16_LOCKABLE,d3ddm) == TRUE )
	{
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16_LOCKABLE;
	}
	else if ( UVUnwrapDeviceCreateTestDepth(pD3D,D3DFMT_D24S8,d3ddm) == TRUE )
	{
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	}
	else if ( UVUnwrapDeviceCreateTestDepth(pD3D,D3DFMT_D24X8,d3ddm) == TRUE )
	{
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	}
	else if ( UVUnwrapDeviceCreateTestDepth(pD3D,D3DFMT_D16,d3ddm) == TRUE )
	{
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	}
	else if ( UVUnwrapDeviceCreateTestDepth(pD3D,D3DFMT_D15S1,d3ddm) == TRUE )
	{
		d3dpp.AutoDepthStencilFormat = D3DFMT_D15S1;
	}
	else
	{
		PANIC_IF(TRUE,"Couldnt find a suitable z-depth format. You may need to change your desktop display format (from 32bit to 16bit, for instance) for this game to work.\n\nAlternatively, check for updates of your video card drivers." );
		// Try with the swapeffect changed
	    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	}

	hr = pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWindow,
	                              D3DCREATE_SOFTWARE_VERTEXPROCESSING/*|D3DCREATE_MULTITHREADED*/,
					                          &d3dpp, &pDevice );

	if ( hr == D3DERR_OUTOFVIDEOMEMORY )
	{
				// TODO Error

	}
	else if ( pDevice )
	{	
		
		// Clear the backbuffer and the zbuffer
		pDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0, 1.0f, 0 );
		pDevice->Present( NULL, NULL, NULL, NULL );
			
		hr = pDevice->Reset( &d3dpp );

		// If failed, try without lockable stencil buffer
		if ( FAILED( hr ) )
		{
			// TODO Error
		}

		pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	    pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	    pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

		pDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
							     0, 1.0f, 0 );
		pDevice->Present( NULL, NULL, NULL, NULL );

		D3DXMATRIX matTrans;
		D3DXMatrixIdentity( &matTrans );
		// Set-up the matrix for the desired transformation.
		pDevice->SetTransform( D3DTS_TEXTURE0, &matTrans );
		pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	}
	else
	{
		// TODO Error
	}

    return( pDevice );
}

void	UVUnwrapRenderSet2dTransform( void )
{
D3DXMATRIX Ortho2D;	
D3DXMATRIX Identity;
	D3DXMatrixOrthoLH(&Ortho2D, (float)(mnUVRenderBufferW), (float)(mnUVRenderBufferH), 0.0f, 1.0f);
	D3DXMatrixIdentity(&Identity);

	mpUVUnwrapD3DDevice->SetTransform(D3DTS_VIEW, &Identity);
	Identity._22 = -1.0f;
	Identity._41 = (float)-(mnUVRenderBufferW/2);
	Identity._42 = (float)+(mnUVRenderBufferH/2);
	mpUVUnwrapD3DDevice->SetTransform(D3DTS_WORLD, &Identity);
	mpUVUnwrapD3DDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D);
}


void	UVUnwrapRenderInitState( void )
{
	UVUnwrapRenderSet2dTransform();

	mpUVUnwrapD3DDevice->SetFVF( D3DFVF_FLATVERTEX );
    mpUVUnwrapD3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	mpUVUnwrapD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE);
    mpUVUnwrapD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	mpUVUnwrapD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE );
    mpUVUnwrapD3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE);
    mpUVUnwrapD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

}

void	UVUnwrapRenderCreateBuffers( void )
{
	if( FAILED( mpUVUnwrapD3DDevice->CreateVertexBuffer( NUM_QUAD_VERTICES * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  D3DPOOL_DEFAULT, &mpQuadVertexBuffer, NULL ) ) )
	{
		PANIC_IF( TRUE, "Couldnt create Textured Overlay Vertex buffer");
	}

	if( FAILED( mpUVUnwrapD3DDevice->CreateVertexBuffer( NUM_LINE_VERTICES * sizeof(FLATVERTEX),
													  D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_FLATVERTEX,
													  D3DPOOL_DEFAULT, &mpLineVertexBuffer, NULL ) ) )
	{
		PANIC_IF( TRUE, "Couldnt create Textured Overlay Vertex buffer");
	}


//	mpQuadVertexBuffer
//	mpLineVertexBuffer
}

void	UVUnwrapLoadTexture( const char* szFilename )
{
LPDIRECT3DDEVICE9		pMainDevice = InterfaceGetD3DDevice();
	
	InterfaceSetD3DDevice( mpUVUnwrapD3DDevice );
	mpMapTexture = InterfaceLoadTextureDX( szFilename, FALSE, FALSE );
	InterfaceSetD3DDevice( pMainDevice );
}



typedef struct
{
	short	nX;
	short	nY;
	short	nWidth;
	short	nHeight;

	uint32	ulCol;
	float	fU1;
	float	fU2;
	float	fV1;
	float	fV2;

} TEXTURED_RECT_DEF;

FLATVERTEX*		mpLineLockedBuffer = NULL;

int		mnLinesAdded = 0;

void		UVWrapRenderAddLine( int X, int Y, int X2, int Y2, uint32 ulCol )
{
	if ( !mpLineLockedBuffer )
	{
		if( FAILED( mpLineVertexBuffer->Lock( 0, 0, (void**)&mpLineLockedBuffer, D3DLOCK_DISCARD ) ) )
		{
			PANIC_IF(TRUE, "Vertex lock failed in addtexoverlay" );
			return;
		}
		mnLinesAdded = 0;
	}

	mpLineLockedBuffer->x = (float)( X );
	mpLineLockedBuffer->y = (float)( Y );
	mpLineLockedBuffer->z = 1.0f;
	mpLineLockedBuffer->color = ulCol;
	mpLineLockedBuffer->tu = 0.0f;
	mpLineLockedBuffer->tv = 0.0f;
	mpLineLockedBuffer++;

	mpLineLockedBuffer->x = (float)( X2 );
	mpLineLockedBuffer->y = (float)( Y2 );
	mpLineLockedBuffer->z = 1.0f;
	mpLineLockedBuffer->color = ulCol;
	mpLineLockedBuffer->tu = 1.0f;
	mpLineLockedBuffer->tv = 1.0f;
	mpLineLockedBuffer++;

	mnLinesAdded++;

}


void		UVWrapRenderAddQuad( LPDIRECT3DVERTEXBUFFER9 pVertexBuffer,TEXTURED_RECT_DEF* pxRectDef )
{
FLATVERTEX*		pVertices;

	if( FAILED( pVertexBuffer->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD ) ) )
	{
		PANIC_IF(TRUE, "Vertex lock failed in addtexoverlay" );
	}

	/** First tri **/
	pVertices->x = (float)( pxRectDef->nX );
	pVertices->y = (float)( pxRectDef->nY );
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU1;
	pVertices->tv = pxRectDef->fV1;
	pVertices++;

	pVertices->x = (float)( pxRectDef->nX + pxRectDef->nWidth );
	pVertices->y = (float)( pxRectDef->nY );
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
	pVertices->tv = pxRectDef->fV1;
	pVertices++;

	pVertices->x = (float)( pxRectDef->nX );
	pVertices->y = (float)( pxRectDef->nY + pxRectDef->nHeight );
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU1;
	pVertices->tv = pxRectDef->fV2;
	pVertices++;
	
	//** Second tri **
	pVertices->x = (float)( pxRectDef->nX + pxRectDef->nWidth );
	pVertices->y = (float)( pxRectDef->nY );
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
	pVertices->tv = pxRectDef->fV1;
	pVertices++;
	
	pVertices->x = (float)( pxRectDef->nX + pxRectDef->nWidth );
	pVertices->y = (float)( pxRectDef->nY + pxRectDef->nHeight );
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU2;
	pVertices->tv = pxRectDef->fV2;
	pVertices++;
	
	pVertices->x = (float)( pxRectDef->nX );
	pVertices->y = (float)( pxRectDef->nY + pxRectDef->nHeight );
	pVertices->z = 1.0f;
	pVertices->color = pxRectDef->ulCol;
	pVertices->tu = pxRectDef->fU1;
	pVertices->tv = pxRectDef->fV2;
	pVertices++;

	pVertexBuffer->Unlock();
}

void	UVUnwrapRenderTexture( void )
{
int		nDrawHowMany = 2;

//mpQuadVertexBuffer

	if ( ( mpMapTexture ) &&
		 ( mpQuadVertexBuffer ) )
	{ 
	TEXTURED_RECT_DEF		rect;

		mpUVUnwrapD3DDevice->SetTexture( 0, mpMapTexture );
		mpUVUnwrapD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		mpUVUnwrapD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		mpUVUnwrapD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		
		rect.fU1 = m_MappingView.fTopLeftU;
		rect.fV1 = m_MappingView.fTopLeftV;
		rect.fU2 = m_MappingView.fBotRightU;
		rect.fV2 = m_MappingView.fBotRightV;

		mfUVUnwrapViewScale = m_MappingView.fBotRightU - m_MappingView.fTopLeftU; 

		// TODO - Scale the texture when we're viewing more than 0.0f -> 1.0f
		if ( ( rect.fU2 - rect.fU1 > 1.0f ) ||
			 ( rect.fV2 - rect.fV1 > 1.0f ) )
		{
		int		nX;
		int		nY;

			UVUnwrapRenderMappingGetCoord( 0.0f, 0.0f, &nX, &nY );
			rect.nX = (short)nX;
			rect.nY = (short)nY;

			UVUnwrapRenderMappingGetCoord( 1.0f, 1.0f, &nX, &nY );
			rect.nWidth = (short)nX - rect.nX;
			rect.nHeight = (short)nY - rect.nY;
		
			rect.fU1 = 0.0f;
			rect.fV1 = 0.0f;
			rect.fU2 = 1.0f;
			rect.fV2 = 1.0f;
		}
		else
		{
			rect.nX = 0;//m_MappingView.nTopLeftX;
			rect.nY = 0;//m_MappingView.nTopLeftY;
			rect.nWidth = mnUVRenderBufferW;
			rect.nHeight = mnUVRenderBufferH;
		}
		rect.ulCol = 0xFFFFFFFF;
		UVWrapRenderAddQuad( mpQuadVertexBuffer, &rect );

		mpUVUnwrapD3DDevice->SetStreamSource( 0, mpQuadVertexBuffer, 0, sizeof(FLATVERTEX) );
		HRESULT hr = mpUVUnwrapD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, nDrawHowMany );
		if ( hr != D3D_OK )
		{
		int		i = 0;
			i++;
		}
	}
}





void	UVUnwrapRenderMappingShowFace( MAPPING_LIST* pMapping )
{
float	fU, fV;
int		x, y;
int		x2, y2;
float	afCoordList[8];
int		abSelections[3];
int		edgeLoop;
uint32	ulVertCol;


	afCoordList[0] = pMapping->u1;
	afCoordList[1] = pMapping->v1;
	afCoordList[2] = pMapping->u2;
	afCoordList[3] = pMapping->v2;
	afCoordList[4] = pMapping->u3;
	afCoordList[5] = pMapping->v3;
	afCoordList[6] = pMapping->u1;
	afCoordList[7] = pMapping->v1;
		
	abSelections[0] = pMapping->nFlags & 0x1;
	abSelections[1] = pMapping->nFlags & 0x2;
	abSelections[2] = pMapping->nFlags & 0x4;


	for ( edgeLoop = 0; edgeLoop < 3; edgeLoop++ )
	{
		fU = afCoordList[(edgeLoop*2)];
		fV = afCoordList[(edgeLoop*2)+1];
		UVUnwrapRenderMappingGetCoord( fU, fV, &x, &y );
		fU = afCoordList[((edgeLoop+1)*2)];
		fV = afCoordList[((edgeLoop+1)*2)+1];
		UVUnwrapRenderMappingGetCoord( fU, fV, &x2, &y2 );

		UVWrapRenderAddLine( x, y, x2, y2, 0xD0FFFFFF );

		ulVertCol = 0xD0f0f0f0;
		if ( abSelections[edgeLoop] != 0 )
		{
			ulVertCol = 0xD0D04010;
		}
		UVWrapRenderAddLine( x-2, y-2, x-2, y+2, ulVertCol );
		UVWrapRenderAddLine( x-2, y-2, x+2, y-2, ulVertCol );
		UVWrapRenderAddLine( x+2, y-2, x+2, y+2, ulVertCol );
		UVWrapRenderAddLine( x-2, y+2, x+2, y+2, ulVertCol );
		UVWrapRenderAddLine( x-3, y-3, x-3, y+3, 0xD0000000 );
		UVWrapRenderAddLine( x-3, y-3, x+3, y-3, 0xD0000000 );
		UVWrapRenderAddLine( x+3, y-3, x+3, y+3, 0xD0000000 );
		UVWrapRenderAddLine( x-3, y+3, x+3, y+3, 0xD0000000 );
	}
}

void	UVUnwrapRenderUI( void )
{
	if ( mbSelectionBoxDisplay )
	{
		UVWrapRenderAddLine( (int)mxUVSelectionBox1.x, (int)mxUVSelectionBox1.y, (int)mxUVSelectionBox1.x, (int)mxUVSelectionBox2.y, 0xC0D0B060 );
		UVWrapRenderAddLine( (int)mxUVSelectionBox1.x, (int)mxUVSelectionBox1.y, (int)mxUVSelectionBox2.x, (int)mxUVSelectionBox1.y, 0xC0D0B060 );
		UVWrapRenderAddLine( (int)mxUVSelectionBox2.x, (int)mxUVSelectionBox1.y, (int)mxUVSelectionBox2.x, (int)mxUVSelectionBox2.y, 0xC0D0B060 );
		UVWrapRenderAddLine( (int)mxUVSelectionBox1.x, (int)mxUVSelectionBox2.y, (int)mxUVSelectionBox2.x, (int)mxUVSelectionBox2.y, 0xC0D0B060 );
	}
}
	
void	UVUnwrapRenderMapping( void )
{
int		loop;
MAPPING_LIST*		pMapping = &mpMappingList[0];

	for ( loop = 0; loop < mnMappingNumFaces; loop++ )
	{
		UVUnwrapRenderMappingShowFace(pMapping);
		pMapping++;
	}
	
	UVUnwrapRenderUI();

	if ( mpLineLockedBuffer )
	{
		mpLineVertexBuffer->Unlock();
		mpLineLockedBuffer = NULL;

		mpUVUnwrapD3DDevice->SetTexture( 0, NULL );
		mpUVUnwrapD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		mpUVUnwrapD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		mpUVUnwrapD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	    mpUVUnwrapD3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	    mpUVUnwrapD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE);
	    mpUVUnwrapD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
//		mpUVUnwrapD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );

		mpUVUnwrapD3DDevice->SetStreamSource( 0, mpLineVertexBuffer, 0, sizeof(FLATVERTEX) );
	    HRESULT hr = mpUVUnwrapD3DDevice->DrawPrimitive( D3DPT_LINELIST, 0, mnLinesAdded );
		mnLinesAdded = 0;
	}

}


void	UVUnwrapRender( void )
{
	if ( mpUVUnwrapD3DDevice )
	{
		mpUVUnwrapD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, (D3DCOLOR)0, 1.0f, 0 );

		mpUVUnwrapD3DDevice->BeginScene();
		UVUnwrapRenderTexture();
		UVUnwrapRenderMapping();
		mpUVUnwrapD3DDevice->EndScene();

		mpUVUnwrapD3DDevice->Present( NULL, NULL, NULL, NULL );
	}

}

void	UVSelectFirstVertInRegion( float u1, float v1, float u2, float v2 )
{
int		loop;
MAPPING_LIST*		pMapping = &mpMappingList[0];
BOOL	bShiftHeld = FALSE;
BOOL	bCtrlHeld = FALSE;

	if ( SysCheckKeyState( KEY_SHIFT ) )
	{
		bShiftHeld = TRUE;
	}
	else if ( SysCheckKeyState( KEY_CTRL ) )
	{
		bCtrlHeld = TRUE;
	}

	if ( ( !bShiftHeld ) &&
		 ( !bCtrlHeld ) )
	{
		for ( loop = 0; loop < mnMappingNumFaces; loop++ )
		{
			// Clear existing selections
			pMapping->nFlags = 0;
			pMapping++;
		}
	}

	pMapping = &mpMappingList[0];
	for ( loop = 0; loop < mnMappingNumFaces; loop++ )
	{
		if ( ( pMapping->u1 >= u1 ) &&
			 ( pMapping->u1 <= u2 ) &&
			 ( pMapping->v1 >= v1 ) &&
			 ( pMapping->v1 <= v2 ) )
		{
			if ( bCtrlHeld )
			{
				pMapping->nFlags &= ~1;
			}
			else
			{
				pMapping->nFlags |= 1;
			}
			return;
		}
		if ( ( pMapping->u2 >= u1 ) &&
			 ( pMapping->u2 <= u2 ) &&
			 ( pMapping->v2 >= v1 ) &&
			 ( pMapping->v2 <= v2 ) )
		{
			if ( bCtrlHeld )
			{
				pMapping->nFlags &= ~2;
			}
			else
			{
				pMapping->nFlags |= 2;
			}
			return;
		}

		if ( ( pMapping->u3 >= u1 ) &&
			 ( pMapping->u3 <= u2 ) &&
			 ( pMapping->v3 >= v1 ) &&
			 ( pMapping->v3 <= v2 ) )
		{
			if ( bCtrlHeld )
			{
				pMapping->nFlags &= ~4;
			}
			else
			{
				pMapping->nFlags |= 4;
			}
			return;
		}
		pMapping++;
	}


}

void	UVSelectAllVertsInRegion( float u1, float v1, float u2, float v2 )
{
int		loop;
MAPPING_LIST*		pMapping = &mpMappingList[0];
BOOL	bShiftHeld = FALSE;
BOOL	bCtrlHeld = FALSE;

	if ( SysCheckKeyState( KEY_SHIFT ) )
	{
		bShiftHeld = TRUE;
	}
	else if ( SysCheckKeyState( KEY_CTRL ) )
	{
		bCtrlHeld = TRUE;
	}

	if ( ( !bShiftHeld ) &&
		 ( !bCtrlHeld ) )
	{
		for ( loop = 0; loop < mnMappingNumFaces; loop++ )
		{
			// Clear existing selections
			pMapping->nFlags = 0;
			pMapping++;
		}
	}

	pMapping = &mpMappingList[0];
	for ( loop = 0; loop < mnMappingNumFaces; loop++ )
	{
		if ( ( pMapping->u1 >= u1 ) &&
			 ( pMapping->u1 <= u2 ) &&
			 ( pMapping->v1 >= v1 ) &&
			 ( pMapping->v1 <= v2 ) )
		{
			if ( bCtrlHeld )
			{
				pMapping->nFlags &= ~1;
			}
			else
			{
				pMapping->nFlags |= 1;
			}
		}
		if ( ( pMapping->u2 >= u1 ) &&
			 ( pMapping->u2 <= u2 ) &&
			 ( pMapping->v2 >= v1 ) &&
			 ( pMapping->v2 <= v2 ) )
		{
			if ( bCtrlHeld )
			{
				pMapping->nFlags &= ~2;
			}
			else
			{
				pMapping->nFlags |= 2;
			}
		}
		if ( ( pMapping->u3 >= u1 ) &&
			 ( pMapping->u3 <= u2 ) &&
			 ( pMapping->v3 >= v1 ) &&
			 ( pMapping->v3 <= v2 ) )
		{
			if ( bCtrlHeld )
			{
				pMapping->nFlags &= ~4;
			}
			else
			{
				pMapping->nFlags |= 4;
			}
		}
		pMapping++;
	}
}


void	UVUnwrapOnLeftButtonUp( void )
{
float	u1, u2;
float	v1, v2;
float	swap;
BOOL	bIsClick = FALSE;

	if ( ( mxUVMousePos.x >= 0 ) &&
		 ( mxUVMousePos.x < mnUVRenderBufferW ) &&
		 ( mxUVMouseDownPos.x >= 0 ) &&
		 ( mxUVMouseDownPos.x < mnUVRenderBufferW ) &&
		 ( mxUVMousePos.y >= 0 ) &&
		 ( mxUVMousePos.y < mnUVRenderBufferH ) &&
		 ( mxUVMouseDownPos.y >= 0 ) &&
		 ( mxUVMouseDownPos.y < mnUVRenderBufferH ) )
	{
		if ( ( mxUVMouseDownPos.x == mxUVMousePos.x ) &&
			 ( mxUVMouseDownPos.y == mxUVMousePos.y ) )
		{
			mxUVMousePos.x -= 3;
			mxUVMousePos.y -= 3;
			mxUVMouseDownPos.x += 3;
			mxUVMouseDownPos.y += 3;
			bIsClick = TRUE;
		}
			 
		UVUnwrapRenderMappingGetUV( (int)mxUVMouseDownPos.x, (int)mxUVMouseDownPos.y, &u1, &v1 );
		UVUnwrapRenderMappingGetUV( (int)mxUVMousePos.x, (int)mxUVMousePos.y, &u2, &v2 );
		if ( u1 > u2 )
		{
			swap = u1;
			u1 = u2;
			u2 = swap;
		}
		if ( v1 > v2 )
		{
			swap = v1;
			v1 = v2;
			v2 = swap;
		}

		switch( m_UVControlMode )
		{
		case ZOOM_BOX:
			m_MappingView.fTopLeftU = u1;
			m_MappingView.fTopLeftV = v1;
			m_MappingView.fBotRightU = u2;
			m_MappingView.fBotRightV = v2;
			break;
		default:
			if ( mbIsMakingSelection )
			{
				if ( bIsClick )
				{
					UVSelectFirstVertInRegion( u1, v1, u2, v2 );
				}
				else
				{
					UVSelectAllVertsInRegion( u1, v1, u2, v2 );
				}
			}
			break;
		}
	}
}

VECT	mxUVLastMousePos;
HCURSOR		m_ArrowCursor;
HCURSOR		m_MoveCursor;

HCURSOR		m_CurrentCursor = 0;

void	UVUnwrapUpdateCursor( int code )
{
HCURSOR		newCursor = m_CurrentCursor;

	switch( code )
	{
	case 0:
		newCursor = m_ArrowCursor;
		break;
	case 1:
		switch ( m_UVControlMode )
		{
		case MOVEXY:
			newCursor = m_MoveCursor;
			break;
		case MOVEX:
			newCursor = m_MoveCursor;
			break;
		case MOVEY:
			newCursor = m_MoveCursor;
			break;
		}
	}

//	if ( newCursor != m_CurrentCursor )
//	{
		m_CurrentCursor = newCursor;
		SetCursor( m_CurrentCursor );
//	}
}


/***************************************************************************
 * Function    : UVUnwrapDlgProc
 * Params      :
 * Returns     :
 * Description :
 ***************************************************************************/
LRESULT CALLBACK UVUnwrapDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
//int		nVal;
//LPNMHDR		pNotifyHeader;
//int		nSelected;
POINTS points;

	switch (message)
	{
	case WM_INITDIALOG:
		m_ArrowCursor = LoadCursor( NULL, IDC_ARROW );
		m_MoveCursor = LoadCursor( NULL, IDC_SIZEALL );
		return TRUE;
	case WM_PAINT:
		UVUnwrapRender();
		break;
    case WM_MOUSEMOVE:
		points = MAKEPOINTS(lParam);

		mxUVMousePos.x = (float)points.x - 11;
		mxUVMousePos.y = (float)points.y - 45;

		if ( mbUVLeftMouseDown == TRUE )
		{
		VECT	mxMouseDelta;

			mxMouseDelta.x = mxUVMousePos.x - mxUVLastMousePos.x;
			mxMouseDelta.y = mxUVMousePos.y - mxUVLastMousePos.y;

			UVUnwrapLeftMouseDownMove( mxMouseDelta.x, mxMouseDelta.y );
			UVUnwrapRender();
			mxUVLastMousePos = mxUVMousePos;

			if ( ( m_UVControlMode == MOVEXY ) ||
				  ( m_UVControlMode == MOVEX ) ||
				  ( m_UVControlMode == MOVEY ) )
			{
				if ( !mbSelectionBoxDisplay )
				{
					UVUnwrapUpdateCursor( 1 );
				}
			}
		}
		else if ( ( m_UVControlMode == MOVEXY ) ||
				  ( m_UVControlMode == MOVEX ) ||
				  ( m_UVControlMode == MOVEY ) )
		{
			if ( UVUnwrapWasMouseOverSelectedVertex( (int)mxUVMousePos.x, (int)mxUVMousePos.y ) )
			{
				UVUnwrapUpdateCursor( 1 );
			}
			else
			{
				UVUnwrapUpdateCursor( 0 );
			}
		}
		else
		{
			UVUnwrapUpdateCursor( 0 );
		}
		break;
	case WM_LBUTTONUP:
		UVUnwrapUpdateCursor( 0 );
		mbSelectionBoxDisplay = FALSE;
		mbUVLeftMouseDown = FALSE;
		UVUnwrapOnLeftButtonUp();
		UVUnwrapRender();
		break;
	case WM_RBUTTONUP:
		mbSelectionBoxDisplay = FALSE;
//		mboRightMouseDown = FALSE;
		break;
	case 0x020A:		// WM_MOUSEWHEEL - for some reason i cant get it to include..
		{
		float	fOffset; 
		short	wShortParam = HIWORD(wParam);
		float	fScaleUnit = 1.0f;
		int		nDelta;

			nDelta = (int)( wShortParam );
			fOffset = (float)( nDelta ) * -0.001f;
			fOffset += 1.0f;
			if ( fOffset < 0.5f )
			{
				fOffset = 0.5f;
			}


			// View is centered at 0.5f,0.5f

			float	fViewCentreX = 0.5f;
			float	fViewCentreY = 0.5f;

			m_MappingView.fTopLeftU = fViewCentreX - ((fViewCentreX - m_MappingView.fTopLeftU) * fOffset);
			m_MappingView.fTopLeftV = fViewCentreY - ((fViewCentreY - m_MappingView.fTopLeftV) * fOffset);
			m_MappingView.fBotRightU = fViewCentreX + ((m_MappingView.fBotRightU - fViewCentreX) * fOffset);
			m_MappingView.fBotRightV = fViewCentreY + ((m_MappingView.fBotRightV - fViewCentreY) * fOffset);
			UVUnwrapRender();
		}
		break;
	case WM_CHAR:
		switch ( (short)( wParam ) )
		{
		case 'X':
		case 'x':
			switch( m_UVControlMode )
			{
			case MOVEXY:
			default:
				UVUnwrapSetControlMode( MOVEX );
				break;
			case MOVEX:
				UVUnwrapSetControlMode( MOVEXY );
				break;
			}
			break;
		case 'Y':
		case 'y':
			switch( m_UVControlMode )
			{
			case MOVEXY:
			default:
				UVUnwrapSetControlMode( MOVEY );
				break;
			case MOVEX:
				UVUnwrapSetControlMode( MOVEXY );
				break;
			}
			break;
		}
		break;
	case WM_MBUTTONUP:
		mbSelectionBoxDisplay = FALSE;
//		mboMidMouseDown = FALSE;
		break;
	case WM_LBUTTONDOWN:
		mbUVLeftMouseDown = TRUE;
		points = MAKEPOINTS(lParam);
		mxUVMouseDownPos.x = (float)points.x - 11;
		mxUVMouseDownPos.y = (float)points.y - 45;
		mxUVLastMousePos = mxUVMouseDownPos;
		memcpy( mpToolStartMappingList, mpMappingList, sizeof(MAPPING_LIST) * mnMappingNumFaces );

		UVUnwrapOnLeftMouseDown();
		UVUnwrapRender();
		break;
	case WM_MBUTTONDOWN:
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDOK:
				UVUnwrapWindowShutdown();
				EndDialog(hDlg, LOWORD(wParam));
				break;
			case IDC_RESETVIEW:
				UVUnwrapResetView();
				break;
			case IDC_CHECK1:
				UVUnwrapSetControlMode( ZOOM_BOX );
				break;
			case IDC_CHECK11:
				UVUnwrapSetControlMode( SCALEXY );
				break;
			case IDC_CHECK12:
				UVUnwrapSetControlMode( SCALEX );
				break;
			case IDC_CHECK13:	
				// mirror..
				break;
			case IDC_CHECK14:
				UVUnwrapSetControlMode( SCALEY );
				break;
			case IDC_CHECK3:
				UVUnwrapSetControlMode( MOVEX );
				break;
			case IDC_CHECK4:
				UVUnwrapSetControlMode( MOVEY );
				break;
			case IDC_CHECK5:
				UVUnwrapSetControlMode( MOVEXY );
				break;
			case IDC_CHECK6:
				UVUnwrapSetControlMode( ROTATE );
				break;
			case IDC_CHECK7:
				// todo - these should be button-like
				UVUnwrapSetControlMode( MIRROR );
				break;
			case IDC_CHECK8:
				// todo - these should be button-like
				UVUnwrapSetControlMode( FLIP );
				break;
			case ID_UV_CANCEL:
				mpSceneObject->ApplyMappingListUVs( mpOriginalMappingList, mnMappingNumFaces );
				UVUnwrapWindowShutdown();
				EndDialog(hDlg, LOWORD(wParam));
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;

	case WM_CLOSE:
		UVUnwrapWindowShutdown();
		EndDialog(hDlg, LOWORD(wParam));
		return( 0 );
	case WM_DESTROY:
		return(0);
	default:
		break;
	}

	return( FALSE );
}

void	UVUnwrapGetMappingList( void )
{
	mnMappingNumFaces = mpSceneObject->GetNumFacesSelected();
	if ( mnMappingNumFaces > 0 )
	{
		mpMappingList = new MAPPING_LIST[ mnMappingNumFaces ];
		mpSceneObject->GetSelectedMappingList( mpMappingList );

		mpOriginalMappingList = new MAPPING_LIST[ mnMappingNumFaces ];
		memcpy( mpOriginalMappingList, mpMappingList, sizeof(MAPPING_LIST) * mnMappingNumFaces );

		mpToolStartMappingList = new MAPPING_LIST[ mnMappingNumFaces ];
		memcpy( mpToolStartMappingList, mpMappingList, sizeof(MAPPING_LIST) * mnMappingNumFaces );
		
	}
}


void	UVUnwrapWindowInit( CSceneObject* pSceneObject, int nMaterialAttrib, void* parentWindowHandle  )
{
	mpSceneObject = pSceneObject;
	if ( mpSceneObject )
	{
	TEXTURE_HANDLE	hTexture;

		mpSceneObject->EnableSelectedFaceHighlight( false );
		// If using the default override texture, rather than specific materials in the atm
		if ( nMaterialAttrib == -1 )
		{
			hTexture = ModelConvGetOverrideTexture();
			if ( hTexture != NOTFOUND )
			{
				EngineExportTexture( hTexture, "temp.dat", 0 );
			}
			else
			{
				// No texture.. so we can't UV map things..
				SysMessageBox( "No texture has been selected", "Unable to use UV edit", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
				return;
			}
		}
		else
		{
			hTexture = mpSceneObject->GetTextureHandle( nMaterialAttrib );
			if ( hTexture == NOTFOUND )
			{
				hTexture = ModelConvGetOverrideTexture();
			}

			if ( hTexture != NOTFOUND )
			{
				EngineExportTexture( hTexture, "temp.dat", 0 );
			}
			else
			{
				// No texture.. so we can't UV map things..
				SysMessageBox( "No texture has been selected", "Unable to use UV edit", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
				return;
			}
		}
	}

	mhwndUVUnwrapMain = CreateDialog(ghInstance, (LPCTSTR)IDD_UV_UNWRAP, NULL, (DLGPROC)UVUnwrapDlgProc );
	ShowWindow( mhwndUVUnwrapMain, SW_SHOW );
	UpdateWindow( mhwndUVUnwrapMain );

	mpUVUnwrapD3DDevice = UVUnwrapWindowCreateDevice(GetDlgItem(mhwndUVUnwrapMain,IDC_DISPLAY_FRAME) );

	UVUnwrapRenderInitState();
	UVUnwrapRenderCreateBuffers();
	UVUnwrapLoadTexture( "temp.dat" );
	UVUnwrapGetMappingList();
	
	UVUnwrapRender();
	UVUnwrapSetControlMode(MOVEXY);
}
