
#include <windows.h>
#include "../resource.h"

#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../LibCode/Engine/ModelRendering.h"
#include "../../LibCode/Engine/Loader.h"

#include "../Tools/RecalcNormals.h"
#include "../Tools/MeshManipulate.h"

#include "../ModelConverter.h"

VECT	axCuboidCornerPositions[] = 
{
	{ -1.0f, -1.0f, -1.0f },		// 0 - back left bottom
	{  1.0f, -1.0f, -1.0f },		// 1 - back right bottom
	{ -1.0f,  1.0f, -1.0f },		// 2 - front left bottom
	{  1.0f,  1.0f, -1.0f },		// 3 - front right bottom

	{ -1.0f, -1.0f,  1.0f },		// 4 - back left top
	{  1.0f, -1.0f,  1.0f },		// 5 - back right top
	{ -1.0f,  1.0f,  1.0f },		// 6 - front left top
	{  1.0f,  1.0f,  1.0f },		// 7 - front right top
};

ushort	auwCuboidIndices[] = 
{
	0, 2, 1,   
	1, 2, 3,   

	0, 6, 2,	
	0, 4, 6,	

	1, 3, 7,	
	1, 7, 5,	
	0, 5, 4,
	0, 1, 5,

	2, 6, 7,
	2, 7, 3,
	4, 7, 6,	
	4, 5, 7,	
};


void		AddPrimitiveCreateCuboid( int nDivisionsX, int nDivisionsY, int nDivisionsZ )
{
int		nNewHandle = ModelRenderGetNextHandle();
MODEL_RENDER_DATA*		pxModelData;
CUSTOMVERTEX*		pVertexBuffer;
CUSTOMVERTEX*		pVertexBufferBase;
ushort*				puwIndexBuffer;
float				fScale = 1.0f;
//int				nLoop;
int				nTotalNumVerts;
int				nTotalNumFaces;
int				nFaceLoop;
int				nWidthLoop;
int				nHeightLoop;
VECT			xWidthSep;
VECT			xHeightSep;
VECT			xBase;
VECT			xVertPos;
int				nHeightDivs;
int				nWidthDivs;
int				nIndexRowStart;
int				nIndexCount;

	nTotalNumVerts = ( (nDivisionsX + 1) * (nDivisionsY + 1) ) * 2;
	nTotalNumVerts += ( (nDivisionsX + 1) * (nDivisionsZ + 1) ) * 2;
	nTotalNumVerts += ( (nDivisionsY + 1) * (nDivisionsZ + 1) ) * 2;

	nTotalNumFaces = ( ( nDivisionsX * nDivisionsY ) * 2 ) * 2;
	nTotalNumFaces += ( ( nDivisionsX * nDivisionsZ ) * 2 ) * 2;
	nTotalNumFaces += ( ( nDivisionsY * nDivisionsZ ) * 2 ) * 2;

	pxModelData = maxModelRenderData + nNewHandle;
	
	ModelConvInitialiseBlankModel( pxModelData, nTotalNumVerts, nTotalNumFaces );

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (BYTE**)&pVertexBufferBase );
	pVertexBuffer = pVertexBufferBase;

	// Create vertices
	for ( nFaceLoop = 0; nFaceLoop < 6; nFaceLoop++ )
	{
		memset( &xWidthSep, 0, sizeof( VECT ) );
		memset( &xHeightSep, 0, sizeof( VECT ) );

		switch( nFaceLoop )
		{
		case 0:	// top face..
			xBase.x = -1.0f;
			xBase.y = -1.0f;
			xBase.z = 1.0f;
			xWidthSep.x = 2.0f / nDivisionsX;
			xHeightSep.y = 2.0f / nDivisionsY;	
			nWidthDivs = nDivisionsX + 1;
			nHeightDivs = nDivisionsY + 1;
			break;
		case 1:	// Right face
			xBase.x = 1.0f;
			xBase.y = 1.0f;
			xBase.z = 1.0f;
			xWidthSep.y = -2.0f / nDivisionsY;
			xHeightSep.z = -2.0f / nDivisionsZ;	
			nWidthDivs = nDivisionsY + 1;
			nHeightDivs = nDivisionsZ + 1;
			break;
		case 2:	// bottom face
			xBase.x = 1.0f;
			xBase.y = -1.0f;
			xBase.z = -1.0f;
			xWidthSep.x = -2.0f / nDivisionsX;
			xHeightSep.y = 2.0f / nDivisionsY;	
			nWidthDivs = nDivisionsX + 1;
			nHeightDivs = nDivisionsY + 1;
			break;
		case 3:		// Left face
			xBase.x = -1.0f;
			xBase.y = -1.0f;
			xBase.z = 1.0f;
			xWidthSep.y = 2.0f / nDivisionsY;
			xHeightSep.z = -2.0f / nDivisionsZ;	
			nWidthDivs = nDivisionsY + 1;
			nHeightDivs = nDivisionsZ + 1;
			break;
		case 4:		// front face
			xBase.x = 1.0f;
			xBase.y = -1.0f;
			xBase.z = 1.0f;
			xWidthSep.x = -2.0f / nDivisionsX;
			xHeightSep.z = -2.0f / nDivisionsZ;	
			nWidthDivs = nDivisionsX + 1;
			nHeightDivs = nDivisionsZ + 1;
			break;
		case 5:		// back face
			xBase.x = -1.0f;
			xBase.y = 1.0f;
			xBase.z = 1.0f;
			xWidthSep.x = 2.0f / nDivisionsX;
			xHeightSep.z = -2.0f / nDivisionsZ;	
			nWidthDivs = nDivisionsX + 1;
			nHeightDivs = nDivisionsZ + 1;
			break;
		}

		for ( nHeightLoop = 0; nHeightLoop < nHeightDivs; nHeightLoop++ )
		{
			xVertPos = xBase;
			for ( nWidthLoop = 0; nWidthLoop < nWidthDivs; nWidthLoop++ )
			{
				pVertexBuffer->position = xVertPos;
				pVertexBuffer->color = 0xFFFFFFFF;
				pVertexBuffer->tu = 0.0f;
				pVertexBuffer->tv = 0.0f;
				pVertexBuffer++;
				VectAdd( &xVertPos, &xVertPos, &xWidthSep );
			}
			VectAdd( &xBase, &xBase, &xHeightSep );
		}
	}

	RenderingComputeBoundingBox( pVertexBufferBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
	RenderingComputeBoundingSphere( pVertexBufferBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );

	pxModelData->pxBaseMesh->UnlockVertexBuffer();

	// Now generate indices
	pxModelData->pxBaseMesh->LockIndexBuffer( 0, (BYTE**)&puwIndexBuffer );

	nIndexRowStart = 0;
	nIndexCount = 0;

	for ( nFaceLoop = 0; nFaceLoop < 6; nFaceLoop++ )
	{
		switch( nFaceLoop )
		{
		case 0:	// top face..
			nWidthDivs = nDivisionsX;
			nHeightDivs = nDivisionsY;
			break;
		case 1:	// Right face
			nWidthDivs = nDivisionsY;
			nHeightDivs = nDivisionsZ;
			break;
		case 2:	// bottom face
			nWidthDivs = nDivisionsX;
			nHeightDivs = nDivisionsY;
			break;
		case 3:		// Left face
			nWidthDivs = nDivisionsY;
			nHeightDivs = nDivisionsZ;
			break;
		case 4:		// front face
			nWidthDivs = nDivisionsX;
			nHeightDivs = nDivisionsZ;
			break;
		case 5:		// back face
			nWidthDivs = nDivisionsX;
			nHeightDivs = nDivisionsZ;
			break;
		}

		for ( nHeightLoop = 0; nHeightLoop < nHeightDivs; nHeightLoop++ )
		{
			nIndexCount = nIndexRowStart;
			for ( nWidthLoop = 0; nWidthLoop < nWidthDivs; nWidthLoop++ )
			{
				puwIndexBuffer[0] = (ushort)( nIndexCount );
				puwIndexBuffer[1] = (ushort)( nIndexCount + 1 );
				puwIndexBuffer[2] = (ushort)( nIndexCount + (nWidthDivs + 1 ) );
				puwIndexBuffer += 3;

				puwIndexBuffer[0] = (ushort)( nIndexCount + 1 );
				puwIndexBuffer[1] = (ushort)( nIndexCount + (nWidthDivs + 1 ) + 1 );
				puwIndexBuffer[2] = (ushort)( nIndexCount + (nWidthDivs + 1 ) );
				puwIndexBuffer += 3;

				nIndexCount++;
			}
			nIndexRowStart += nWidthDivs + 1;
		}
		nIndexRowStart += nWidthDivs + 1;
	}

	pxModelData->pxBaseMesh->UnlockIndexBuffer();
	
	// Calc normals
	ModelConvFixNormals( nNewHandle, FLAT_FACES );

	ModelConvSetCurrentModel( nNewHandle );
}


void		AddPrimitiveCreateCuboidSimple( int nDivisionsX, int nDivisionsY, int nDivisionsZ )
{
int		nNewHandle = ModelRenderGetNextHandle();
MODEL_RENDER_DATA*		pxModelData;
CUSTOMVERTEX*		pVertexBuffer;
CUSTOMVERTEX*		pVertexBufferBase;
ushort*				puwIndexBuffer;
float				fScale = 1.0f;
int				nLoop;

	pxModelData = maxModelRenderData + nNewHandle;
	
	ModelConvInitialiseBlankModel( pxModelData, 8, 12 );

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (BYTE**)&pVertexBufferBase );

	pVertexBuffer = pVertexBufferBase;
	for( nLoop = 0; nLoop < 8; nLoop++ )
	{
		pVertexBuffer->position = axCuboidCornerPositions[nLoop];
		pVertexBuffer->color = 0xFFFFFFFF;
		pVertexBuffer->tu = 0.0f;
		pVertexBuffer->tv = 0.0f;

		pVertexBuffer++;
	}

	RenderingComputeBoundingBox( pVertexBufferBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
	RenderingComputeBoundingSphere( pVertexBufferBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );

	pxModelData->pxBaseMesh->UnlockVertexBuffer();
	
	pxModelData->pxBaseMesh->LockIndexBuffer( 0, (BYTE**)&puwIndexBuffer );
	for( nLoop = 0; nLoop < 12*3; nLoop++ )
	{
		puwIndexBuffer[nLoop] = auwCuboidIndices[nLoop];
	}

	pxModelData->pxBaseMesh->UnlockIndexBuffer();
	

	ModelConvSeparateVerts( nNewHandle );

	ModelConvFixNormals( nNewHandle, FLAT_FACES );

	ModelConvSetCurrentModel( nNewHandle );
}


LRESULT CALLBACK AddPrimitiveDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
//MODEL_RENDER_DATA*	pxModelData;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText( hDlg, IDC_NUMVERTSX, "1" );
		SetDlgItemText( hDlg, IDC_NUMVERTSY, "1" );
		SetDlgItemText( hDlg, IDC_NUMVERTSZ, "1" );
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDOK:
				{
				char	acString[256];
//				int		nVal;
				int		nDivisionsX, nDivisionsY, nDivisionsZ;

					GetDlgItemText( hDlg, IDC_NUMVERTSX, acString, 256 );
					nDivisionsX = strtol( acString, NULL, 10 );
					GetDlgItemText( hDlg, IDC_NUMVERTSY, acString, 256 );
					nDivisionsZ = strtol( acString, NULL, 10 );
					GetDlgItemText( hDlg, IDC_NUMVERTSZ, acString, 256 );
					nDivisionsY = strtol( acString, NULL, 10 );

					AddPrimitiveCreateCuboid( nDivisionsX, nDivisionsY, nDivisionsZ );
					EndDialog(hDlg, 0);
				}
				break;
			case IDCANCEL:
				EndDialog(hDlg, 0);
				break;
			}
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return( 0 );
	case WM_DESTROY:
		return(0);
	default:
		break;
	}
	return( FALSE );
}


void		AddPrimitiveCuboid( void )
{
	DialogBox(ghInstance, (LPCTSTR)IDD_ADDPRIMITIVE, NULL, (DLGPROC)AddPrimitiveDlg );		
}