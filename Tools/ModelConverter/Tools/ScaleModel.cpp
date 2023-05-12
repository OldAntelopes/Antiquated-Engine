#include <stdio.h>
#include <windows.h>
#include "../resource.h"

#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../../Pub/LibCode/Engine/ModelRendering.h"		// Temp.. shouldnt be doing this :)

#include "../ModelConverter.h"
#include "MoveModel.h"
#include "ScaleModel.h"

HWND	mhwndScaleDialog;

int		mnLandScale = 115;

float	mfLastScaleTU = 1.0f;
float	mfLastScaleX = 100.0f;
float	mfLastScaleY = 100.0f;
float	mfLastScaleZ = 100.0f;

float		mfOneTileSize = 115.0f/256.0f;

void ModelConvScaleToTileSize( int nMode )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVertBase;
int		nVertLoop;
int		nVertsInMesh;
float	fScale;
int		nHandle = ModelConvGetCurrentModel();
float	fRange;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		if ( nMode == 0 )
		{
			fRange = pxModelData->xStats.xBoundMax.x - pxModelData->xStats.xBoundMin.x;
		}
		else
		{
			fRange = pxModelData->xStats.xBoundMax.y - pxModelData->xStats.xBoundMin.y;
		}

		fScale = mfOneTileSize / fRange;
		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
			pxVertBase = pxVertices;
			if ( nMode == 0 )
			{
				for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
				{
					pxVertices->position.x *= fScale;
					pxVertices++;
				}
			}
			else
			{
				for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
				{
					pxVertices->position.y *= fScale;
					pxVertices++;
				}
			}
			RenderingComputeBoundingBox( pxVertBase, nVertsInMesh,&pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
			RenderingComputeBoundingSphere( pxVertBase, nVertsInMesh, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );

			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
	ModelConverterDisplayFrame( TRUE );
}


void ModelConvScaleModel( int nModelHandle, float fMetres )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVertBase;
int		nVertLoop;
int		nVertsInMesh;
float	fScale	= fMetres / ModelConvGetScaleUnit();
float	fHeight;
int		nHandle = nModelHandle;
VECT*	pxVerts;
int		nVertsTotal;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;

		fHeight = pxModelData->xStats.xBoundMax.z - pxModelData->xStats.xBoundMin.z;
		fScale = (fScale / fHeight);

		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
			pxVertBase = pxVertices;
			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->position.x *= fScale;
				pxVertices->position.y *= fScale;
				pxVertices->position.z *= fScale;
				pxVertices++;
			}
			RenderingComputeBoundingBox( pxVertBase, nVertsInMesh,&pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
			RenderingComputeBoundingSphere( pxVertBase, nVertsInMesh, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );

			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}

		if ( pxModelData->pxVertexKeyframes != NULL )
		{
			pxVerts = pxModelData->pxVertexKeyframes;
			nVertsTotal = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumVertices;
			for ( nVertLoop = 0; nVertLoop < nVertsTotal; nVertLoop++ )
			{
				pxVerts->x *= fScale;
				pxVerts->y *= fScale;
				pxVerts->z *= fScale;
				pxVerts++;
			}
		}

		if ( pxModelData->xVertTurretData.nModelHandle != NOTFOUND )
		{
			ModelConvScaleModelFree( pxModelData->xVertTurretData.nModelHandle, fScale*100.0f,fScale*100.0f,fScale*100.0f );
		}
		if ( pxModelData->xHorizTurretData.nModelHandle != NOTFOUND )
		{
			ModelConvScaleModelFree( pxModelData->xHorizTurretData.nModelHandle, fScale*100.0f,fScale*100.0f,fScale*100.0f);
		}
		if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
		{
			ModelConvScaleModelFree( pxModelData->xLowLODAttachData.nModelHandle, fScale*100.0f,fScale*100.0f,fScale*100.0f);
		}
		if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
		{
			ModelConvScaleModelFree( pxModelData->xMedLODAttachData.nModelHandle, fScale*100.0f,fScale*100.0f,fScale*100.0f);
		}
		if ( pxModelData->xWheel1AttachData.nModelHandle != NOTFOUND )
		{
			ModelConvScaleModelFree( pxModelData->xWheel1AttachData.nModelHandle, fScale*100.0f,fScale*100.0f,fScale*100.0f);
		}
		if ( pxModelData->xWheel3AttachData.nModelHandle != NOTFOUND )
		{
			ModelConvScaleModelFree( pxModelData->xWheel3AttachData.nModelHandle, fScale*100.0f,fScale*100.0f,fScale*100.0f);
		}
		if ( pxModelData->xCollisionAttachData.nModelHandle != NOTFOUND )
		{
			ModelConvScaleModelFree( pxModelData->xCollisionAttachData.nModelHandle, fScale*100.0f,fScale*100.0f,fScale*100.0f);
		}
		
	}

	ModelConverterDisplayFrame( TRUE );
}


// x,y,z are percentage values..
void ModelConvScaleModelFree( int nModelHandle, float fX, float fY, float fZ )
{
	ModelScale( nModelHandle, fX * 0.01f, fY * 0.01f, fZ * 0.01f );
	ModelConverterDisplayFrame( TRUE );
}

void ScaleXToTileSize( void )
{
float	fSize;
float	fTileSize = (float)( mnLandScale ) / 256.0f;
MODEL_RENDER_DATA*		pxModelData;
int		nModelHandle = ModelConvGetCurrentModel();

	pxModelData = &maxModelRenderData[ nModelHandle ];

	fSize = pxModelData->xStats.xBoundMax.x - pxModelData->xStats.xBoundMin.x;
	fSize = (fTileSize / fSize) * 100.0f;
	ModelConvScaleModelFree( nModelHandle, fSize, 100.0f, 100.0f );
}

void ScaleYToTileSize( void )
{
float	fSize;
float	fTileSize = (float)( mnLandScale ) / 256.0f;
MODEL_RENDER_DATA*		pxModelData;
int		nModelHandle = ModelConvGetCurrentModel();

	pxModelData = &maxModelRenderData[ nModelHandle ];

	fSize = pxModelData->xStats.xBoundMax.y - pxModelData->xStats.xBoundMin.y;
	fSize = (fTileSize / fSize) * 100.0f;
	ModelConvScaleModelFree( nModelHandle, 100.0f, fSize, 100.0f );
}


/***************************************************************************
 * Function    : ModelConvFixBuildingScale
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void ModelConvFixBuildingScale( int nHandle )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
int		nVertLoop;
int		nVertsInMesh;

	if ( nHandle != NOTFOUND )
	{
		pxModelData += nHandle;
		
		if ( pxModelData->pxBaseMesh != NULL )
		{
			nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

			for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
			{
				pxVertices->position.x /= 0.0035f;
				pxVertices->position.y /= 0.0035f;
				pxVertices->position.z /= 0.0035f;

				pxVertices->position.x *= 0.0013f;
				pxVertices->position.y *= 0.0013f;
				pxVertices->position.z *= 0.0013f;

				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
	ModelConverterDisplayFrame( TRUE );
}


/***************************************************************************
 * Function    : ScaleDlgProc
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
LRESULT CALLBACK ScaleDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;

	switch (message)
	{
	case WM_INITDIALOG:
		{
		char	acString[256];
		float	fScale;
		MODEL_STATS*	pxStats;
		int		nModelHandle = ModelConvGetCurrentModel();
		float	fScaleUnit = ModelConvGetScaleUnit();

			mhwndScaleDialog = hDlg;
			sprintf( acString, "%.3f",mfLastScaleTU );
			SetDlgItemText( hDlg, IDC_EDIT1, acString );
			sprintf( acString, "%.3f",mfLastScaleX );
			SetDlgItemText( hDlg, IDC_EDIT2, acString );
			sprintf( acString, "%.3f",mfLastScaleY );
			SetDlgItemText( hDlg, IDC_EDIT3, acString );
			sprintf( acString, "%.3f",mfLastScaleZ );
			SetDlgItemText( hDlg, IDC_EDIT4, acString );
			
			sprintf( acString, "%d",mnLandScale );
			SetDlgItemText( hDlg, IDC_LAND_SCALE, acString );

			pxStats = ModelGetStats(nModelHandle);
			fScale = pxStats->xBoundMax.z - pxStats->xBoundMin.z;
			EnableWindow( GetDlgItem( hDlg, IDC_EDIT2 ), FALSE );
			EnableWindow( GetDlgItem( hDlg, IDC_EDIT3 ), FALSE );
			EnableWindow( GetDlgItem( hDlg, IDC_EDIT4 ), FALSE );

			fScale *= fScaleUnit;
			SetDlgItemText( hDlg, IDC_SCALE_UNITS, "m high" );
			sprintf( acString, "Current model height = %.5fm", fScale );
			SetDlgItemText( hDlg, IDC_CURRENT_SCALE, acString );

			sprintf( acString, "X : %.5f to %.5f = %.5fm", pxStats->xBoundMin.x, pxStats->xBoundMax.x, (pxStats->xBoundMax.x - pxStats->xBoundMin.x)*fScaleUnit);
			SetDlgItemText( hDlg, IDC_CURRENT_X_SCALE, acString );
			sprintf( acString, "Y : %.5f to %.5f = %.5fm", pxStats->xBoundMin.y, pxStats->xBoundMax.y, (pxStats->xBoundMax.y - pxStats->xBoundMin.y)*fScaleUnit );
			SetDlgItemText( hDlg, IDC_CURRENT_Y_SCALE, acString );
			sprintf( acString, "Z : %.5f to %.5f = %.5fm", pxStats->xBoundMin.z, pxStats->xBoundMax.z, (pxStats->xBoundMax.z - pxStats->xBoundMin.z)*fScaleUnit );
			SetDlgItemText( hDlg, IDC_CURRENT_Z_SCALE, acString );

			SendDlgItemMessage( hDlg, IDC_RADIO1, BM_SETCHECK, BST_CHECKED, 0 );
			SendDlgItemMessage( hDlg, IDC_RADIO2, BM_SETCHECK, BST_UNCHECKED, 0 );
		}
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
				float	fVal;
				char	acString[256];
				int		nModelHandle = ModelConvGetCurrentModel();

					GetDlgItemText( hDlg, IDC_EDIT1, acString, 256 );
					fVal = (float)( strtod( acString, NULL ) );
					if ( fVal > 0.0f )
					{
					int	nVal;
						nVal = SendDlgItemMessage( hDlg, IDC_RADIO1, BM_GETCHECK, 0, 0 );
						if ( nVal == 1 )
						{
							mfLastScaleTU = fVal;
							ModelConvScaleModel( nModelHandle, fVal  );
						}
						else
						{
						float	fXScalePerc;
						float	fYScalePerc;
						float	fZScalePerc;

							GetDlgItemText( hDlg, IDC_EDIT2, acString, 256 );
							fXScalePerc = (float)( strtod( acString, NULL ) );
							GetDlgItemText( hDlg, IDC_EDIT3, acString, 256 );
							fYScalePerc = (float)( strtod( acString, NULL ) );
							GetDlgItemText( hDlg, IDC_EDIT4, acString, 256 );
							fZScalePerc = (float)( strtod( acString, NULL ) );
							mfLastScaleX = fXScalePerc;
							mfLastScaleY = fYScalePerc;
							mfLastScaleZ = fZScalePerc;
							ModelConvScaleModelFree( nModelHandle, fXScalePerc, fYScalePerc, fZScalePerc );
						}
					}
				}
				EndDialog(hDlg, 0);
				break;
			case IDC_RADIO2:
				EnableWindow( GetDlgItem( hDlg, IDC_EDIT1 ), FALSE );
				EnableWindow( GetDlgItem( hDlg, IDC_EDIT2 ), TRUE );
				EnableWindow( GetDlgItem( hDlg, IDC_EDIT3 ), TRUE );
				EnableWindow( GetDlgItem( hDlg, IDC_EDIT4 ), TRUE );
				EnableWindow( GetDlgItem( hDlg, IDC_BUTTON3 ), TRUE );
				EnableWindow( GetDlgItem( hDlg, IDC_BUTTON4 ), TRUE );
				SendDlgItemMessage( hDlg, IDC_RADIO1, BM_SETCHECK, BST_UNCHECKED, 0 );
				break;
			case IDC_RADIO1:
				EnableWindow( GetDlgItem( hDlg, IDC_EDIT1 ), TRUE );
				EnableWindow( GetDlgItem( hDlg, IDC_EDIT2 ), FALSE );
				EnableWindow( GetDlgItem( hDlg, IDC_EDIT3 ), FALSE );
				EnableWindow( GetDlgItem( hDlg, IDC_EDIT4 ), FALSE );
				EnableWindow( GetDlgItem( hDlg, IDC_BUTTON3 ), FALSE );
				EnableWindow( GetDlgItem( hDlg, IDC_BUTTON4 ), FALSE );
				SendDlgItemMessage( hDlg, IDC_RADIO2, BM_SETCHECK, BST_UNCHECKED, 0 );
				break;
			case IDC_BUTTON3:
				{
				char	acString[256];
					GetDlgItemText( hDlg, IDC_LAND_SCALE, acString, 256 );
					mnLandScale = strtol( acString, NULL, 10 );
					if ( mnLandScale < 50 ) 
					{
						mnLandScale = 50;
						sprintf( acString, "%d",mnLandScale );
						SetDlgItemText( hDlg, IDC_LAND_SCALE, acString );
					}
					else if ( mnLandScale > 2000 )
					{
						mnLandScale = 2000;
						sprintf( acString, "%d",mnLandScale );
						SetDlgItemText( hDlg, IDC_LAND_SCALE, acString );
					}
					ScaleXToTileSize();				
				}
				break;
			case IDC_BUTTON4:
				{
				char	acString[256];
					GetDlgItemText( hDlg, IDC_LAND_SCALE, acString, 256 );
					mnLandScale = strtol( acString, NULL, 10 );
					if ( mnLandScale < 50 ) 
					{
						mnLandScale = 50;
						sprintf( acString, "%d",mnLandScale );
						SetDlgItemText( hDlg, IDC_LAND_SCALE, acString );
					}
					else if ( mnLandScale > 2000 )
					{
						mnLandScale = 2000;
						sprintf( acString, "%d",mnLandScale );
						SetDlgItemText( hDlg, IDC_LAND_SCALE, acString );
					}
					ScaleYToTileSize();				
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



void			ScaleDialogActivate( void )
{
	DialogBox(ghInstance, (LPCTSTR)IDD_SCALE_MODEL_DIALOG, NULL, (DLGPROC)ScaleDlgProc );
}


void	ModelConvScaleTo15mGrid( int hModel )
{
MODEL_STATS*		pxStats;
float				fXSize;
float				fYSize;
float				fRescaleX;
float				fRescaleY;
float				fRescaleZ;

	pxStats = ModelGetStats(hModel);

	fXSize = pxStats->xBoundMax.x - pxStats->xBoundMin.x;
	fYSize = pxStats->xBoundMax.y - pxStats->xBoundMin.y;

	if ( ( fXSize > 0.0f ) &&
		 ( fYSize > 0.0f ) )
	{
		fRescaleX = 15.0f / fXSize;
		fRescaleY = 15.0f / fYSize;
		fRescaleZ = (fRescaleX + fRescaleY) * 0.5f;

		// ModelConvScaleModelFree takes values as a percentage scale
		fRescaleX *= 100.0f;
		fRescaleY *= 100.0f;
		fRescaleZ *= 100.0f;
		ModelConvScaleModelFree( hModel, fRescaleX, fRescaleY, fRescaleZ );
//		ModelConvCentreHorizontally();
	}
}
