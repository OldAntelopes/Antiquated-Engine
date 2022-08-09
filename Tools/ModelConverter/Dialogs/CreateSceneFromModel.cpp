#include <stdio.h>
#include <windows.h>
#include <CommCtrl.h>

#include <StandardDef.h>
#include <System.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>
#include "../LibCode/Engine/ModelFiles.h"
#include "../LibCode/Engine/ModelRendering.h"	
#include "../LibCode/Engine/ModelMaterialData.h"		

#include "../resource.h"
#include "../ModelConverter.h" 
#include "CreateSceneFromModel.h"

HWND		mhModelToSceneDlg = NULL;



void		CreateGridSceneFromModel( int hModel, int nNumGridsX, int nNumGridsY, int nNumGridsZ )
{
MODEL_STATS*		pxModelStats;
MODEL_RENDER_DATA*	pxModelData;
MODEL_RENDER_DATA*	pxNewModelData;
int*				pnFaceGridList;
int*				pnGridFaceCounts;
VECT				xBoundMin, xBoundMax;
int					nLoop;
int					nNewModelHandle;
unsigned int		unIndex1, unIndex2, unIndex3;
unsigned int*		punIndices;
ushort*				puwIndices;
CUSTOMVERTEX*		pxVertices;
CUSTOMVERTEX*		pxVert1;
CUSTOMVERTEX*		pxVert2;
CUSTOMVERTEX*		pxVert3;
VECT				xMidPoint;
int					nNumFaces;
VECT				xGridSize;
int					nGridX, nGridY, nGridZ;
int					nTotalNumGrids = nNumGridsX * nNumGridsY * nNumGridsZ;
EngineMesh*			pNewMesh;
unsigned int*		punNewIndices;
ushort*				puwNewIndices;
CUSTOMVERTEX*		pxNewVertices;
int					nNumNewFaces;
int					nNewLoop;
char				acString[256];

	pxModelData = &maxModelRenderData[ hModel ];
	pxModelStats = ModelGetStats( hModel );

	nNumFaces = pxModelStats->nNumIndices / 3;
	xBoundMin = pxModelStats->xBoundMin;
	xBoundMax = pxModelStats->xBoundMax;

	xGridSize.x = (xBoundMax.x - xBoundMin.x) / nNumGridsX;
	xGridSize.y = (xBoundMax.y - xBoundMin.y) / nNumGridsY;
	xGridSize.z = (xBoundMax.z - xBoundMin.z) / nNumGridsZ;

	// Create a list of ints, one for each face
	pnFaceGridList = (int*)( malloc( nNumFaces * sizeof( int ) ) );
	pnGridFaceCounts = (int*)( malloc( nTotalNumGrids * sizeof( int ) ) );
	memset( pnGridFaceCounts, 0, nTotalNumGrids * sizeof( int ) );

	// Parse through the model faces, find midpoint, and allocate the face to a grid square. (Store the number of the grid square in the list above)
	pxModelData->pxBaseMesh->LockVertexBuffer(kLock_ReadOnly, (byte**)( &pxVertices ) );

	if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
	{
		pxModelData->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &punIndices ) );
	}
	else
	{
		pxModelData->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &puwIndices ) );
	}

	for ( nLoop = 0; nLoop < nNumFaces; nLoop ++ )
	{
		if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
		{
			unIndex1 = punIndices[ (nLoop*3) ];
			unIndex2 = punIndices[ (nLoop*3)+1 ];
			unIndex3 = punIndices[ (nLoop*3)+2 ];
		}
		else
		{
			unIndex1 = puwIndices[ (nLoop*3) ];
			unIndex2 = puwIndices[ (nLoop*3)+1 ];
			unIndex3 = puwIndices[ (nLoop*3)+2 ];
		}
		pxVert1 = pxVertices + unIndex1;
		pxVert2 = pxVertices + unIndex2;
		pxVert3 = pxVertices + unIndex3;

		xMidPoint = pxVert1->position;
		VectAdd( &xMidPoint, &xMidPoint, &pxVert2->position );
		VectAdd( &xMidPoint, &xMidPoint, &pxVert3->position );
		VectScale( &xMidPoint, &xMidPoint, (1.0f / 3.0f) );
		xMidPoint.x = xMidPoint.x - xBoundMin.x;
		xMidPoint.y = xMidPoint.y - xBoundMin.y;
		xMidPoint.z = xMidPoint.z - xBoundMin.z;

		nGridX = (int)( xMidPoint.x / xGridSize.x );
		nGridY = (int)( xMidPoint.y / xGridSize.y );
		nGridZ = 0;// (int)( xMidPoint.z / xGridSize.z );

		pnFaceGridList[ nLoop ] = nGridX + (nGridY * nNumGridsX) + (nGridZ * nNumGridsY * nNumGridsX);
		pnGridFaceCounts[ pnFaceGridList[ nLoop ] ] = pnGridFaceCounts[ pnFaceGridList[ nLoop ] ] + 1;
	}

	// For each grid... 
	for ( nLoop = 0; nLoop < nTotalNumGrids; nLoop++ )
	{
		nNumNewFaces = pnGridFaceCounts[ nLoop ];
		if ( nNumNewFaces > 0 )
		{
		int		nNewFaceNum = 0;

			//	Create a model with the appropriate number of faces
			EngineCreateMesh( nNumNewFaces, nNumNewFaces * 3, &pNewMesh, 0 );

			pNewMesh->LockVertexBuffer( kLock_Normal, (byte**)( &pxNewVertices ) );

			if ( pNewMesh->Is32BitIndexBuffer() == TRUE )
			{
				pNewMesh->LockIndexBuffer(kLock_Normal, (byte**)( &punNewIndices ) );
			}
			else
			{
				pNewMesh->LockIndexBuffer(kLock_Normal, (byte**)( &puwNewIndices ) );
			}

			for ( nNewLoop = 0; nNewLoop < nNumFaces; nNewLoop++ )
			{
				// Is this face for this grid?
				if ( pnFaceGridList[ nNewLoop ] == nLoop )
				{
					// Create new indices
					if ( pNewMesh->Is32BitIndexBuffer() == TRUE )
					{
						punNewIndices[ (nNewFaceNum*3) ] = (nNewFaceNum*3);
						punNewIndices[ (nNewFaceNum*3)+1 ] = (nNewFaceNum*3)+1;
						punNewIndices[ (nNewFaceNum*3)+2 ] = (nNewFaceNum*3)+2;		
					}
					else
					{
						puwNewIndices[ (nNewFaceNum*3) ] = (ushort)( (nNewFaceNum*3) );
						puwNewIndices[ (nNewFaceNum*3)+1 ] = (ushort)( (nNewFaceNum*3)+1 );
						puwNewIndices[ (nNewFaceNum*3)+2 ] = (ushort)( (nNewFaceNum*3)+2 );		
					}

					//	Copy vertex data in
					if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
					{
						unIndex1 = punIndices[ (nNewLoop*3) ];
						unIndex2 = punIndices[ (nNewLoop*3)+1 ];
						unIndex3 = punIndices[ (nNewLoop*3)+2 ];
					}
					else
					{
						unIndex1 = puwIndices[ (nNewLoop*3) ];
						unIndex2 = puwIndices[ (nNewLoop*3)+1 ];
						unIndex3 = puwIndices[ (nNewLoop*3)+2 ];
					}

					pxVert1 = pxVertices + unIndex1;
					pxVert2 = pxVertices + unIndex2;
					pxVert3 = pxVertices + unIndex3;
			
					*pxNewVertices++ = *pxVert1;
					*pxNewVertices++ = *pxVert2;
					*pxNewVertices++ = *pxVert3;

					nNewFaceNum++;
				}
			}

			//	[ Create attribute buffer ]
			//  [ Copy attribute data in ]
			//  [ Copy material data from original model]
			//  [ Remap attributes to the materials contained within each grid ]

			// [ Generate LODs ] 
			pNewMesh->UnlockVertexBuffer();
			pNewMesh->UnlockIndexBuffer();

			// Save out as ATM
			nNewModelHandle = ModelRenderGetNextHandle();
			if ( nNewModelHandle != NOTFOUND )
			{
				pxNewModelData = &maxModelRenderData[ nNewModelHandle ];
				pxNewModelData->bModelType = MODEL_TYPE_STATIC_MESH;
				pxNewModelData->xGlobalProperties.bOpacity = 100;
				pxNewModelData->pxBaseMesh = pNewMesh;
//TODO					pxNewModelData->xStats.fBoundSphereRadius
//todo					pxNewModelData->xStats.xBoundSphereCentre 
//todo					pxNewModelData->xStats.xBoundMin 
//todo					pxNewModelData->xStats.xBoundMin 
				pxNewModelData->xStats.nNumIndices = nNumNewFaces * 3;
				pxNewModelData->xStats.nNumMaterials = 0;
				pxNewModelData->xStats.nNumVertices = nNumNewFaces * 3;
				sprintf( acString, "GridSplit%04d.atm", nLoop );
				ModelExportATM( nNewModelHandle, acString, 0, 0 );
				ModelFree( nNewModelHandle );				
				// Add to UIM list
			}

		} // end if nNumNewFaces > 0
	}
	
	pxModelData->pxBaseMesh->UnlockVertexBuffer();
	pxModelData->pxBaseMesh->UnlockIndexBuffer();

	// Save out UIM
	
	// oh and will probably need a progress bar :)
}




/***************************************************************************
 * Function    : ModelToSceneDlg
 * Params      :
 * Returns     :
 * Description :
 ***************************************************************************/
LRESULT CALLBACK ModelToSceneDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
//NMUPDOWN*	lpnmud;
//MODEL_RENDER_DATA*	pxModelData;

	switch (message)
	{
	case WM_INITDIALOG:
		mhModelToSceneDlg = hDlg;
		SetDlgItemText( hDlg, IDC_EDIT1, "8" );
		SetDlgItemText( hDlg, IDC_EDIT2, "8" );
		SetDlgItemText( hDlg, IDC_EDIT3, "1" );
		SendDlgItemMessage( hDlg, IDC_RADIO1, BM_SETCHECK, BST_CHECKED, 0 );
		SendDlgItemMessage( hDlg, IDC_RADIO2, BM_SETCHECK, BST_UNCHECKED, 0 );
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
				char		acBuff[256];
				int			nGridX;
				int			nGridY;
				int			nGridZ;

					GetDlgItemText( hDlg, IDC_EDIT1, acBuff, 255 );
					nGridX = strtol( acBuff, NULL, 10 );
					GetDlgItemText( hDlg, IDC_EDIT2, acBuff, 255 );
					nGridY = strtol( acBuff, NULL, 10 );
					GetDlgItemText( hDlg, IDC_EDIT3, acBuff, 255 );
					nGridZ = strtol( acBuff, NULL, 10 );
					CreateGridSceneFromModel( ModelConvGetCurrentModel(), nGridX, nGridY, nGridZ );
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
