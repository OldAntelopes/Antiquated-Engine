#include <stdio.h>
#include <windows.h>
#include "../resource.h"

#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../../LibCode/Engine/ModelRendering.h"

#include "../ModelConverter.h"
#include "MoveModel.h"

HWND	mhwndMoveDialog;

void ModelConvCentreHorizontally( void )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVertBase;
int		nVertLoop;
int		nVertsInMesh;
int		nHandle = ModelConvGetCurrentModel();
float	fRangeX;
float	fRangeY;
float	fOffsetX;
float	fOffsetY;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		fRangeX = pxModelData->xStats.xBoundMax.x - pxModelData->xStats.xBoundMin.x;
		fRangeY = pxModelData->xStats.xBoundMax.y - pxModelData->xStats.xBoundMin.y;
	
		// work out where the min x and y should be
		fOffsetX = (0.0f - (fRangeX / 2));
		fOffsetY = (0.0f - (fRangeY / 2));

		// work out the changed needed to put the current min x and y where we want em
		fOffsetX = fOffsetX - pxModelData->xStats.xBoundMin.x;
		fOffsetY = fOffsetY - pxModelData->xStats.xBoundMin.y;

		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
			pxVertBase = pxVertices;
			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->position.x += fOffsetX;
				pxVertices->position.y += fOffsetY;
				pxVertices++;
			}
			RenderingComputeBoundingBox( pxVertBase, nVertsInMesh, &pxModelData->xStats.xBoundMin,&pxModelData->xStats.xBoundMax );
			RenderingComputeBoundingSphere( pxVertBase, nVertsInMesh, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );

			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
	ModelConverterDisplayFrame( TRUE );
}


void ModelConvMoveModelFree( int nModelHandle, float fX, float fY, float fZ )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVertBase;
int		nVertLoop;
int		nVertsInMesh;
int		nHandle = nModelHandle;
VECT*	pxVerts;
int		nVertsTotal;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;

		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
			pxVertBase = pxVertices;
			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->position.x += fX;
				pxVertices->position.y += fY;
				pxVertices->position.z += fZ;
				pxVertices++;
			}
			RenderingComputeBoundingBox( pxVertBase, nVertsInMesh,&pxModelData->xStats.xBoundMin,&pxModelData->xStats.xBoundMax );
			RenderingComputeBoundingSphere( pxVertBase, nVertsInMesh, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );

			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}

		if ( pxModelData->pxVertexKeyframes != NULL )
		{
			pxVerts = pxModelData->pxVertexKeyframes;
			nVertsTotal = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumVertices;
			for ( nVertLoop = 0; nVertLoop < nVertsTotal; nVertLoop++ )
			{
				pxVerts->x += fX;
				pxVerts->y += fY;
				pxVerts->z += fZ;
				pxVerts++;
			}
		}
	}
	ModelConverterDisplayFrame( TRUE );
}


/***************************************************************************
 * Function    : MoveDlgProc
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
LRESULT CALLBACK MoveDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;

	switch (message)
	{
	case WM_INITDIALOG:
		{
		char	acString[256];
		//float	fScale;
		int		nModelHandle = ModelConvGetCurrentModel();

			mhwndMoveDialog = hDlg;
			sprintf( acString, "0.0" );
			SetDlgItemText( hDlg, IDC_EDIT2, acString );
			SetDlgItemText( hDlg, IDC_EDIT3, acString );
			SetDlgItemText( hDlg, IDC_EDIT4, acString );

			sprintf( acString, "(Model X Size is %.4f)", ModelGetStats( nModelHandle )->xBoundMax.x - ModelGetStats( nModelHandle )->xBoundMin.x  );
			SetDlgItemText( hDlg, IDC_X_TEXT, acString );
			sprintf( acString, "(Model Y Size is %.4f)", ModelGetStats( nModelHandle )->xBoundMax.y - ModelGetStats( nModelHandle )->xBoundMin.y  );
			SetDlgItemText( hDlg, IDC_Y_TEXT, acString );
			sprintf( acString, "(Model Z Size is %.4f)", ModelGetStats( nModelHandle )->xBoundMax.z - ModelGetStats( nModelHandle )->xBoundMin.z  );
			SetDlgItemText( hDlg, IDC_Z_TEXT, acString );

			SendDlgItemMessage( hDlg, IDC_RADIO2, BM_SETCHECK, BST_CHECKED, 0 );
		}
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case ID_APPLY:
				{
				float	fXOffset;
				float	fYOffset;
				float	fZOffset;
				char	acString[256];
				int		nModelHandle = ModelConvGetCurrentModel();

					GetDlgItemText( hDlg, IDC_EDIT2, acString, 256 );
					fXOffset = (float)( strtod( acString, NULL ) );
					GetDlgItemText( hDlg, IDC_EDIT3, acString, 256 );
					fYOffset = (float)( strtod( acString, NULL ) );
					GetDlgItemText( hDlg, IDC_EDIT4, acString, 256 );
					fZOffset = (float)( strtod( acString, NULL ) );
					ModelConvMoveModelFree( nModelHandle, fXOffset, fYOffset, fZOffset );
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



void ModelConvMove( int nHandle, float fX, float fY, float fZ )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
int		nVertLoop;
int		nVertsInMesh;
VECT*	pxVerts;
int		nVertsTotal;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		nVertsInMesh = pxModelData->xStats.nNumVertices;
		
		if ( pxModelData->pxBaseMesh != NULL )
		{
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->position.x += fX;
				pxVertices->position.y += fY;
				pxVertices->position.z += fZ;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}

		if ( pxModelData->pxVertexKeyframes != NULL )
		{
			pxVerts = pxModelData->pxVertexKeyframes;
			nVertsTotal = pxModelData->xStats.nNumVertKeyframes * nVertsInMesh;
			for ( nVertLoop = 0; nVertLoop < nVertsTotal; nVertLoop++ )
			{
				pxVerts->x += fX;
				pxVerts->y += fY;
				pxVerts->z += fZ;
				pxVerts++;
			}

		}
	}
	ModelConverterDisplayFrame( TRUE );

}



void ModelConvStickToFloor( int nHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
int		nVertLoop;
CUSTOMVERTEX*	pxVertexBase;
int		nVertsInMesh;
float	fLowestZ;
VECT*	pxVerts;
int		nVertsTotal;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		nVertsInMesh = pxModelData->xStats.nNumVertices;
		
		if ( pxModelData->pxBaseMesh != NULL )
		{
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
			pxVertexBase = pxVertices;

			fLowestZ = pxVertices->position.z;

			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				if ( pxVertices->position.z < fLowestZ )
				{
					fLowestZ = pxVertices->position.z;
				}
				pxVertices++;
			}

			pxVertices = pxVertexBase;

			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->position.z -= fLowestZ;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();

			if ( pxModelData->xVertTurretData.nModelHandle != NOTFOUND )
			{
				ModelConvMove( pxModelData->xVertTurretData.nModelHandle, 0.0f, 0.0f, -fLowestZ );
			}
			if ( pxModelData->xHorizTurretData.nModelHandle != NOTFOUND )
			{
				ModelConvMove( pxModelData->xHorizTurretData.nModelHandle, 0.0f, 0.0f, -fLowestZ);
			}
		}

		if ( pxModelData->pxVertexKeyframes != NULL )
		{
			pxVerts = pxModelData->pxVertexKeyframes;
			fLowestZ = pxVerts->z;
			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				if ( pxVerts->z < fLowestZ )
				{
					fLowestZ = pxVerts->z;
				}
				pxVerts += pxModelData->xStats.nNumVertKeyframes;
			}
			pxVerts = pxModelData->pxVertexKeyframes;
			nVertsTotal = pxModelData->xStats.nNumVertKeyframes * nVertsInMesh;
			for ( nVertLoop = 0; nVertLoop < nVertsTotal; nVertLoop++ )
			{
				pxVerts->z -= fLowestZ;
				pxVerts++;
			}

		}
	}
	ModelConverterDisplayFrame( TRUE );

}

void	MoveDialogActivate( void )
{
	DialogBox(ghInstance, (LPCTSTR)IDD_MOVE_MODEL_DIALOG, NULL, (DLGPROC)MoveDlgProc );
}
