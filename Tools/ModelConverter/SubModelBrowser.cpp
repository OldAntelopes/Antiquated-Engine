#include <stdio.h>
#include <windows.h>
#include <CommCtrl.h>
#include <shlobj.h>
#include "resource.h"

#include <d3dx9.h>

#include <StandardDef.h>
#include <System.h>
#include <Rendering.h>
#include <Interface.h>
#include <Engine.h>

#include "../LibCode/Engine/ModelFiles.h"
#include "../LibCode/Engine/ModelRendering.h"
#include "ModelConverter.h"
#include "SubModelBrowser.h"



HWND	mhwndSubModelBrowserDialog;
int		mnCurrentSubModelHandle = NOTFOUND;
eSUBMODEL_LIST		mnCurrentSubModelIndex = kHORIZONTAL_TURRET;


void	SubModelBrowserInitSubModelDisplay( eSUBMODEL_LIST nSubModelNum )
{
int	nModelHandle = ModelConvGetCurrentModel();
int	nSubModelHandle = NOTFOUND;
MODEL_RENDER_DATA*		pxSubModelData = NULL;
char	acBuffer[512];

	mnCurrentSubModelIndex = nSubModelNum;

	if ( nModelHandle != NOTFOUND )
	{
	MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];

		switch( nSubModelNum )
		{
		case kWHEEL1:
			nSubModelHandle = pxModelData->xWheel1AttachData.nModelHandle;
			break;
		case kWHEEL2:
			nSubModelHandle = pxModelData->xWheel3AttachData.nModelHandle;
			break;
		case kHORIZONTAL_TURRET:
			nSubModelHandle = pxModelData->xHorizTurretData.nModelHandle;
			break;
		case kVERTICAL_TURRET:
			nSubModelHandle = pxModelData->xVertTurretData.nModelHandle;
			break;
		case kMEDLOD:
			nSubModelHandle = pxModelData->xMedLODAttachData.nModelHandle;
			break;
		case kLOWLOD:
			nSubModelHandle = pxModelData->xLowLODAttachData.nModelHandle;
			break;
		case kCOLLISION:
			nSubModelHandle = pxModelData->xCollisionAttachData.nModelHandle;
			break;
		}	

		mnCurrentSubModelHandle = nSubModelHandle;

		if ( nSubModelHandle != NOTFOUND )
		{
			pxSubModelData = &maxModelRenderData[ nSubModelHandle ];

			sprintf( acBuffer, "%d polys, %d vertices", pxSubModelData->xStats.nNumIndices/3, pxSubModelData->xStats.nNumVertices );
			EnableWindow( GetDlgItem(mhwndSubModelBrowserDialog, IDC_REMOVE_SUBMODEL), TRUE );
			EnableWindow( GetDlgItem(mhwndSubModelBrowserDialog, IDC_SUBMODEL_EXPORT), TRUE );	
		}
		else
		{
			sprintf( acBuffer, "No submodel set" );
			EnableWindow( GetDlgItem(mhwndSubModelBrowserDialog, IDC_REMOVE_SUBMODEL), FALSE );
			EnableWindow( GetDlgItem(mhwndSubModelBrowserDialog, IDC_SUBMODEL_EXPORT), FALSE );	
		}
		SetDlgItemText( mhwndSubModelBrowserDialog, IDC_MODEL_DETAILS_TEXT, acBuffer );

	}

}

void	SubModelBrowserInitDialog( void )
{

	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Wheel 1 & 2" );
	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Wheel 3 & 4" );
	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Horizontal Turret" );
	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Vertical Turret" );
	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Medium LOD" );
	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Low LOD" );
	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Collision Model" );
	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO1, CB_SETCURSEL, 0, 0 );

	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Display in position" );
	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Display separately" );
	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Hide" );
	SendDlgItemMessage( mhwndSubModelBrowserDialog, IDC_COMBO2, CB_SETCURSEL, 0, 0 );

	SubModelBrowserInitSubModelDisplay( kWHEEL1 );
}

void SubModelEditorExportCurrent( void )
{
	ModelConvSaveAsDialog(mnCurrentSubModelHandle, FALSE );
}

void SubModelEditorChangeCurrent( void )
{
	ModelConvOpenDialog( mnCurrentSubModelIndex );
}

void	SubModelEditorRemoveCurrent( void )
{
int	nModelHandle = ModelConvGetCurrentModel();
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];

	switch( mnCurrentSubModelIndex )
	{
	case kWHEEL1:
		pxModelData->xWheel1AttachData.nModelHandle = NOTFOUND;
		pxModelData->xWheel2AttachData.nModelHandle = NOTFOUND;
		ModelFree( pxModelData->xWheel1AttachData.nModelHandle );
		break;
	case kWHEEL2:
		pxModelData->xWheel3AttachData.nModelHandle = NOTFOUND;
		pxModelData->xWheel4AttachData.nModelHandle = NOTFOUND;
		ModelFree( pxModelData->xWheel3AttachData.nModelHandle );
		break;
	case kHORIZONTAL_TURRET:
		pxModelData->xHorizTurretData.nModelHandle = NOTFOUND;
		ModelFree( pxModelData->xHorizTurretData.nModelHandle );
		break;
	case kVERTICAL_TURRET:
		pxModelData->xVertTurretData.nModelHandle = NOTFOUND;
		ModelFree( pxModelData->xVertTurretData.nModelHandle );
		break;
	case kMEDLOD:
		pxModelData->xMedLODAttachData.nModelHandle = NOTFOUND;
		ModelFree( pxModelData->xMedLODAttachData.nModelHandle );
		break;
	case kLOWLOD:
		pxModelData->xLowLODAttachData.nModelHandle = NOTFOUND;
		ModelFree( pxModelData->xLowLODAttachData.nModelHandle );
		break;
	case kCOLLISION:
		pxModelData->xCollisionAttachData.nModelHandle = NOTFOUND;
		ModelFree( pxModelData->xCollisionAttachData.nModelHandle );
		break;
	}	
	SubModelBrowserInitSubModelDisplay( mnCurrentSubModelIndex );
}


/***************************************************************************
 * Function    : SubModelBrowserDlgProc
 * Params      :
 * Returns     :
 * Description :
 ***************************************************************************/
LRESULT CALLBACK SubModelBrowserDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
int		nVal;

	switch (message)
	{
	case WM_INITDIALOG:
		mhwndSubModelBrowserDialog = hDlg;
		SubModelBrowserInitDialog();
		return TRUE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case EN_CHANGE:
//			MaterialBrowserOnChangesMade();
			break;
		case CBN_SELCHANGE:
			nVal = SendDlgItemMessage( hDlg, LOWORD(wParam), CB_GETCURSEL, 0, 0 );
			switch( LOWORD(wParam) )
			{
			case IDC_COMBO1:
				SubModelBrowserInitSubModelDisplay((eSUBMODEL_LIST)nVal);
				break;
			case IDC_COMBO2:
				switch( nVal )
				{
				case 0:		// Display the submodel normally in the scene
				default:
					ModelConvSetTemporaryDisplayModel( NOTFOUND );
					break;
				case 1:		// Display only the submodel
					ModelConvSetTemporaryDisplayModel( mnCurrentSubModelHandle );
					break;
				case 2:		// Hide the submodel
					break;
				}
			}
			break;

		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDOK:
				ModelConvSetTemporaryDisplayModel(NOTFOUND);
				EndDialog(hDlg, LOWORD(wParam));
				break;
			case IDC_REMOVE_SUBMODEL:
				SubModelEditorRemoveCurrent();
				break;
			case IDC_SUBMODEL_EXPORT:
				SubModelEditorExportCurrent();
				break;
			case IDC_SUBMODEL_CHANGE:
				SubModelEditorChangeCurrent();
				break;
//			case IDC_GLOBAL_CHECK:
//				MaterialBrowserApplyChanges();
//				EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_APPLY ), FALSE );
//				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;

	case WM_CLOSE:
		ModelConvSetTemporaryDisplayModel(NOTFOUND);
		EndDialog(hDlg, LOWORD(wParam));
		return( 0 );
	case WM_DESTROY:
		return(0);
	default:
		break;
	}

	return( FALSE );
}



void		SubModelBrowserInit( HWND hParentDlg )
{
	mhwndSubModelBrowserDialog = CreateDialog(ghInstance, (LPCTSTR)IDD_SUBMODEL_BROWSER, hParentDlg, (DLGPROC)SubModelBrowserDlgProc );

    ShowWindow( mhwndSubModelBrowserDialog, SW_SHOW );
	UpdateWindow( mhwndSubModelBrowserDialog );
}
