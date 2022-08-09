
#include <math.h>
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


void		AddPrimitiveCurvedWall( float fInnerRadius, float fWallDepth, float fWallHeight, int nNumDivisions )
{
int		nNewHandle = ModelRenderGetNextHandle();
MODEL_RENDER_DATA*		pxModelData;
CUSTOMVERTEX*		pVertexBuffer;
CUSTOMVERTEX*		pVertexBufferBase;
ushort*				puwIndexBuffer;
float				fScale = 1.0f;
int				nLoop;
int				nTotalNumVerts;
int				nTotalNumFaces;
VECT			xVertPos;
int				nIndexRowStart;
int				nIndexCount;
float			fAngle;
float			fAngleSep;
VECT			xNormal;
float			fUFlip = 0.0f;

	nTotalNumVerts = nNumDivisions * 4;
	nTotalNumFaces = nNumDivisions * 6;

	pxModelData = maxModelRenderData + nNewHandle;
	
	ModelConvInitialiseBlankModel( pxModelData, nTotalNumVerts, nTotalNumFaces );

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (BYTE**)&pVertexBufferBase );
	pVertexBuffer = pVertexBufferBase;

	fAngle = 0.0f;
	fAngleSep = A360 / nNumDivisions;

	// Create vertices
	for ( nLoop = 0; nLoop < nNumDivisions; nLoop++ )
	{
		xVertPos.x = sinf( fAngle ) * fInnerRadius;
		xVertPos.y = cosf( fAngle ) * fInnerRadius;
		xVertPos.z = 0.0f;
	
		xNormal = xVertPos;
		VectNormalize( &xNormal );
		VectScale( &xNormal,&xNormal, -1.0f );

		pVertexBuffer->position = xVertPos;
		pVertexBuffer->color = 0xFFFFFFFF;
		pVertexBuffer->tu = fUFlip;
		pVertexBuffer->tv = 0.0f;
		pVertexBuffer->normal = xNormal;
		pVertexBuffer++;

		xVertPos.z = fWallHeight;

		pVertexBuffer->position = xVertPos;
		pVertexBuffer->color = 0xFFFFFFFF;
		pVertexBuffer->tu = fUFlip;
		pVertexBuffer->tv = 0.4f;
		pVertexBuffer->normal = xNormal;
		pVertexBuffer++;
		
		xVertPos.x = sinf( fAngle ) * (fInnerRadius + fWallDepth);
		xVertPos.y = cosf( fAngle ) * (fInnerRadius + fWallDepth);
		xVertPos.z = 0.0f;
		VectScale( &xNormal,&xNormal, -1.0f );

		pVertexBuffer->position = xVertPos;
		pVertexBuffer->color = 0xFFFFFFFF;
		pVertexBuffer->tu = fUFlip;
		pVertexBuffer->tv = 1.0f;
		pVertexBuffer->normal = xNormal;
		pVertexBuffer++;

		xVertPos.z = fWallHeight;

		pVertexBuffer->position = xVertPos;
		pVertexBuffer->color = 0xFFFFFFFF;
		pVertexBuffer->tu = fUFlip;
		pVertexBuffer->tv = 0.6f;
		pVertexBuffer->normal = xNormal;
		pVertexBuffer++;

		if ( fUFlip == 0.0f )
		{
			fUFlip = 1.0f;
		}
		else
		{
			fUFlip = 0.0f;
		}

		fAngle += fAngleSep;
	}

	RenderingComputeBoundingBox( pVertexBufferBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
	RenderingComputeBoundingSphere( pVertexBufferBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );

	pxModelData->pxBaseMesh->UnlockVertexBuffer();

	// Now generate indices
	pxModelData->pxBaseMesh->LockIndexBuffer( 0, (BYTE**)&puwIndexBuffer );

	nIndexRowStart = 0;
	nIndexCount = 0;

	for ( nLoop = 0; nLoop < nNumDivisions - 1; nLoop++ )
	{
		puwIndexBuffer[0] = (ushort)( nIndexCount );
		puwIndexBuffer[1] = (ushort)( nIndexCount + 4 );
		puwIndexBuffer[2] = (ushort)( nIndexCount + 1 );
		puwIndexBuffer += 3;

		puwIndexBuffer[0] = (ushort)( nIndexCount + 1 );
		puwIndexBuffer[1] = (ushort)( nIndexCount + 4 );
		puwIndexBuffer[2] = (ushort)( nIndexCount + 5 );
		puwIndexBuffer += 3;

		puwIndexBuffer[0] = (ushort)( nIndexCount + 1 );
		puwIndexBuffer[1] = (ushort)( nIndexCount + 5 );
		puwIndexBuffer[2] = (ushort)( nIndexCount + 3 );
		puwIndexBuffer += 3;

		puwIndexBuffer[0] = (ushort)( nIndexCount + 3 );
		puwIndexBuffer[1] = (ushort)( nIndexCount + 5 );
		puwIndexBuffer[2] = (ushort)( nIndexCount + 7 );
		puwIndexBuffer += 3;

		puwIndexBuffer[0] = (ushort)( nIndexCount + 2 );
		puwIndexBuffer[1] = (ushort)( nIndexCount + 3 );
		puwIndexBuffer[2] = (ushort)( nIndexCount + 6 );
		puwIndexBuffer += 3;

		puwIndexBuffer[0] = (ushort)( nIndexCount + 6 );
		puwIndexBuffer[1] = (ushort)( nIndexCount + 3 );
		puwIndexBuffer[2] = (ushort)( nIndexCount + 7 );
		puwIndexBuffer += 3;

		nIndexCount += 4;
	}

	// Last face connects last set of verts with the first..
	puwIndexBuffer[0] = (ushort)( nIndexCount );
	puwIndexBuffer[1] = (ushort)( 0 );
	puwIndexBuffer[2] = (ushort)( nIndexCount + 1 );
	puwIndexBuffer += 3;

	puwIndexBuffer[0] = (ushort)( nIndexCount + 1 );
	puwIndexBuffer[1] = (ushort)( 0 );
	puwIndexBuffer[2] = (ushort)( 1 );
	puwIndexBuffer += 3;

	puwIndexBuffer[0] = (ushort)( nIndexCount + 1 );
	puwIndexBuffer[1] = (ushort)( 1 );
	puwIndexBuffer[2] = (ushort)( nIndexCount + 3 );
	puwIndexBuffer += 3;

	puwIndexBuffer[0] = (ushort)( nIndexCount + 3 );
	puwIndexBuffer[1] = (ushort)( 1 );
	puwIndexBuffer[2] = (ushort)( 3 );
	puwIndexBuffer += 3;

	puwIndexBuffer[0] = (ushort)( nIndexCount + 2 );
	puwIndexBuffer[1] = (ushort)( nIndexCount + 3 );
	puwIndexBuffer[2] = (ushort)( 2 );
	puwIndexBuffer += 3;

	puwIndexBuffer[0] = (ushort)( 2 );
	puwIndexBuffer[1] = (ushort)( nIndexCount + 3 );
	puwIndexBuffer[2] = (ushort)( 3 );
	puwIndexBuffer += 3;

	pxModelData->pxBaseMesh->UnlockIndexBuffer();
	
	// Calc normals
//	ModelConvFixNormals( nNewHandle, FLAT_FACES );

	ModelConvSetCurrentModel( nNewHandle );
}


LRESULT CALLBACK CurvedWallDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
//MODEL_RENDER_DATA*	pxModelData;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText( hDlg, IDC_INNER_RADIUS, "8.0" );
		SetDlgItemText( hDlg, IDC_WALL_DEPTH, "1.0" );
		SetDlgItemText( hDlg, IDC_WALL_HEIGHT, "2.0" );
		SetDlgItemText( hDlg, IDC_NUM_DIVISIONS, "16" );
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
				float	fInnerRadius;
				float	fWallDepth;
				float	fWallHeight;
				int		nNumDivisions;

					GetDlgItemText( hDlg, IDC_INNER_RADIUS, acString, 256 );
					fInnerRadius = (float)strtod( acString, NULL );
					GetDlgItemText( hDlg, IDC_WALL_DEPTH, acString, 256 );
					fWallDepth = (float)strtod( acString, NULL );
					GetDlgItemText( hDlg, IDC_WALL_HEIGHT, acString, 256 );
					fWallHeight = (float)strtod( acString, NULL );
					GetDlgItemText( hDlg, IDC_NUM_DIVISIONS, acString, 256 );
					nNumDivisions = strtol( acString, NULL, 10 );

					AddPrimitiveCurvedWall( fInnerRadius, fWallDepth, fWallHeight, nNumDivisions );
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


void		AddCurvedWall( void )
{
	DialogBox(ghInstance, (LPCTSTR)IDD_CURVED_WALL, NULL, (DLGPROC)CurvedWallDlg );		
}