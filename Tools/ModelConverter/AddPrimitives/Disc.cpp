
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


void		AddPrimitiveDisc( int nNumDivisions )
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
float			fInnerRadius = 1.0f;

	nTotalNumVerts = nNumDivisions + 1;
	nTotalNumFaces = nNumDivisions;

	pxModelData = maxModelRenderData + nNewHandle;
	
	ModelConvInitialiseBlankModel( pxModelData, nTotalNumVerts, nTotalNumFaces );

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (BYTE**)&pVertexBufferBase );
	pVertexBuffer = pVertexBufferBase;

	fAngle = 0.0f;
	fAngleSep = A360 / nNumDivisions;

	xNormal.x = 0.0f;
	xNormal.y = 0.0f;
	xNormal.z = 1.0f;

	// Initial vert at origin
	xVertPos.x = 0.0f;
	xVertPos.y = 0.0f;
	xVertPos.z = 0.0f;

	pVertexBuffer->position = xVertPos;
	pVertexBuffer->color = 0xFFFFFFFF;
	pVertexBuffer->tu = 0.5f;
	pVertexBuffer->tv = 0.5f;
	pVertexBuffer->normal = xNormal;
	pVertexBuffer++;

	// Create vertices
	for ( nLoop = 0; nLoop < nNumDivisions; nLoop++ )
	{
		xVertPos.x = sinf( fAngle ) * fInnerRadius;
		xVertPos.y = cosf( fAngle ) * fInnerRadius;
		xVertPos.z = 0.0f;
	
		pVertexBuffer->position = xVertPos;
		pVertexBuffer->color = 0xFFFFFFFF;
		pVertexBuffer->tu = 0.5f + (xVertPos.x*0.5f);
		pVertexBuffer->tv = 0.5f + (xVertPos.y*0.5f);
		pVertexBuffer->normal = xNormal;
		pVertexBuffer++;

		fAngle += fAngleSep;
	}

	RenderingComputeBoundingBox( pVertexBufferBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
	RenderingComputeBoundingSphere( pVertexBufferBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );

	pxModelData->pxBaseMesh->UnlockVertexBuffer();

	// Now generate indices
	pxModelData->pxBaseMesh->LockIndexBuffer( 0, (BYTE**)&puwIndexBuffer );

	nIndexRowStart = 0;
	nIndexCount = 0;

	for ( nLoop = 0; nLoop < nNumDivisions; nLoop++ )
	{
		puwIndexBuffer[0] = 0;

		if ( nLoop == nNumDivisions - 1)
		{
			puwIndexBuffer[1] = 1;
		}
		else
		{
			puwIndexBuffer[1] = (ushort)( nLoop + 1 );
		}
		puwIndexBuffer[2] = (ushort)( nLoop );
		puwIndexBuffer += 3;
	}

	pxModelData->pxBaseMesh->UnlockIndexBuffer();
	
	// Calc normals
//	ModelConvFixNormals( nNewHandle, FLAT_FACES );

	ModelConvSetCurrentModel( nNewHandle );
}


LRESULT CALLBACK GenDiscDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
//MODEL_RENDER_DATA*	pxModelData;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText( hDlg, IDC_NUM_DIVISIONS, "64" );
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
				int		nNumDivisions;

					GetDlgItemText( hDlg, IDC_NUM_DIVISIONS, acString, 256 );
					nNumDivisions = strtol( acString, NULL, 10 );

					AddPrimitiveDisc( nNumDivisions );
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


void		AddDisc( void )
{
	DialogBox(ghInstance, (LPCTSTR)IDD_PRIMITIVES_DISC, NULL, (DLGPROC)GenDiscDlg );		
}