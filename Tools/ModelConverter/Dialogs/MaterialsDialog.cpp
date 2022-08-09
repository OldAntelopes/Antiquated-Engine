#include <stdio.h>
#include <windows.h>
#include "../resource.h"

#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../../LibCode/Engine/ModelRendering.h"

#include "../ModelConverter.h"
#include "MaterialsDialog.h"


HWND		mhwndMaterialsDialog;


void	ModelConvMaterialEditInitLists( void )
{
MODEL_RENDER_DATA*	pxModelData;
char	acString[256];
int		nModelHandle = ModelConvGetCurrentModel();

	SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_RESETCONTENT, 0, 0 );
//	sprintf( acString, "<All>", nLoop );
//	SendDlgItemMessage( mhwndMainDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)acString );
	SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"None" );
	SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Blend Alpha with cutoff" );
	SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Additive Alpha" );
	SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Subtractive Alpha" );
	SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Blend Colour" );
	SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Additive Colour" );
	SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Subtractive Colour" );
	SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Blend Alpha" );
	
	if ( nModelHandle != NOTFOUND )
	{
		pxModelData = maxModelRenderData + nModelHandle;
		SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_SETCURSEL, pxModelData->xGlobalProperties.bBlendType, 0 );		
		if ( pxModelData->xGlobalProperties.bNoLighting == 1 )
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_LIGHTING, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_LIGHTING, BM_SETCHECK, BST_UNCHECKED, 0 );
		}

		if ( pxModelData->xGlobalProperties.bBackfaceFlag == 1 )
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_DOUBLE_SIDED_CHECK, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_DOUBLE_SIDED_CHECK, BM_SETCHECK, BST_UNCHECKED, 0 );
		}
		if ( pxModelData->xGlobalProperties.bCollisionOverride == 1 )
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_COLLISION_CHECK, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_COLLISION_CHECK, BM_SETCHECK, BST_UNCHECKED, 0 );
		}
		sprintf( acString, "%.3f", pxModelData->xGlobalProperties.fBounciness );
		SetDlgItemText( mhwndMaterialsDialog, IDC_BOUNCINESS, acString );

		if ( pxModelData->xGlobalProperties.bNoFiltering == 1 )
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_FILTERING, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_FILTERING, BM_SETCHECK, BST_UNCHECKED, 0 );
		}

		if ( pxModelData->xGlobalProperties.bDontClampUVs == 1 )
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_DONT_CLAMP_UVS, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_DONT_CLAMP_UVS, BM_SETCHECK, BST_UNCHECKED, 0 );
		}	

		if ( pxModelData->xGlobalProperties.bNoShadows == 1 )
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_NO_SHADOWS, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_NO_SHADOWS, BM_SETCHECK, BST_UNCHECKED, 0 );
		}
		
		if ( pxModelData->xGlobalProperties.bLargeModelClipping == 1 )
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_LARGE_MODEL_CLIPPING, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_LARGE_MODEL_CLIPPING, BM_SETCHECK, BST_UNCHECKED, 0 );
		}

		if ( pxModelData->xGlobalProperties.bSpriteFlags == 1 )
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_SINGLE_SPRITE_CHECK, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_SINGLE_SPRITE_CHECK, BM_SETCHECK, BST_UNCHECKED, 0 );
		}
		if ( pxModelData->xGlobalProperties.bSpriteFlags == 2 )
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_POLYGON_SPRITES_CHECK, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMaterialsDialog, IDC_POLYGON_SPRITES_CHECK, BM_SETCHECK, BST_UNCHECKED, 0 );
		}

		sprintf( acString, "%d", pxModelData->xGlobalProperties.bOpacity );
		SetDlgItemText( mhwndMaterialsDialog, IDC_EDIT1, acString );
	}
}

void	ModelConvSetVertexAlphas( void )
{
MODEL_RENDER_DATA*	pxModelData;
CUSTOMVERTEX*		pxVertices;
int			nVertLoop;
ulong		ulAlpha;
int		nModelHandle = ModelConvGetCurrentModel();

	if ( nModelHandle != NOTFOUND )
	{
		pxModelData = maxModelRenderData + nModelHandle;
		ulAlpha = (((ulong)(pxModelData->xGlobalProperties.bOpacity) * 255)/100) << 24;

		if ( pxModelData->pxBaseMesh != NULL )
		{
			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

			for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
			{
				pxVertices->color &= 0x00FFFFFF;
				pxVertices->color |= ulAlpha;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
}


/***************************************************************************
 * Function    : ModelConverterMaterialsDlgProc
 * Params      :
 * Returns     :
 * Description : Message handler for the "save changes" dialog box
 ***************************************************************************/
LRESULT CALLBACK ModelConverterMaterialsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
int		nVal;
float	fVal;
	switch (message)
	{
	case WM_INITDIALOG:
		mhwndMaterialsDialog = hDlg;
		ModelConvMaterialEditInitLists();
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case EN_CHANGE:
			{
			int		nSelected;
			char	acString[256];
			MODEL_RENDER_DATA*	pxModelData;
			int		nModelHandle = ModelConvGetCurrentModel();

				nSelected = LOWORD( wParam );
				GetDlgItemText( mhwndMaterialsDialog, nSelected, acString, 256 );

				switch ( nSelected )
				{
				case IDC_EDIT1:
					nVal = strtol( acString, NULL, 10 );
					nVal %= 101;
					if ( nModelHandle != NOTFOUND )
					{
						pxModelData = maxModelRenderData + nModelHandle;
						if ( nVal != pxModelData->xGlobalProperties.bOpacity )
						{
							pxModelData->xGlobalProperties.bOpacity = (byte)( nVal );
							ModelConvSetVertexAlphas();
						}
					}
					break;
				}
			}
			break;
		case CBN_SELCHANGE:
			{
			MODEL_RENDER_DATA*	pxModelData;
			int		nModelHandle = ModelConvGetCurrentModel();

				nVal = SendDlgItemMessage( mhwndMaterialsDialog, IDC_BLENDTYPE_COMBO, CB_GETCURSEL, 0, 0 );
				if ( nModelHandle != NOTFOUND )
				{
					pxModelData = maxModelRenderData + nModelHandle;
					pxModelData->xGlobalProperties.bBlendType = (byte)( nVal );
				}
			}
			break;
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDOK:
				{
				char	acString[256];
				MODEL_RENDER_DATA*	pxModelData;
				int		nModelHandle = ModelConvGetCurrentModel();
	
					GetDlgItemText( mhwndMaterialsDialog, IDC_EDIT1, acString, 256 );
					nVal = strtol( acString, NULL, 10  );
					if ( nModelHandle != NOTFOUND )
					{
						pxModelData = maxModelRenderData + nModelHandle;
						pxModelData->xGlobalProperties.bOpacity = (byte)( nVal );

						nVal = SendDlgItemMessage( mhwndMaterialsDialog, IDC_FILTERING, BM_GETCHECK, 0, 0 );
						if ( nVal == BST_CHECKED )
						{
							pxModelData->xGlobalProperties.bNoFiltering = 1;
						}
						else
						{
							pxModelData->xGlobalProperties.bNoFiltering = 0;
						}
						nVal = SendDlgItemMessage( mhwndMaterialsDialog, IDC_DONT_CLAMP_UVS, BM_GETCHECK, 0, 0 );
						if ( nVal == BST_CHECKED )
						{
							pxModelData->xGlobalProperties.bDontClampUVs = 1;
						}
						else
						{
							pxModelData->xGlobalProperties.bDontClampUVs = 0;
						}
					
						nVal = SendDlgItemMessage( mhwndMaterialsDialog, IDC_DOUBLE_SIDED_CHECK, BM_GETCHECK, 0, 0 );
						if ( nVal == BST_CHECKED )
						{
							pxModelData->xGlobalProperties.bBackfaceFlag = 1;
						}
						else
						{
							pxModelData->xGlobalProperties.bBackfaceFlag = 0;
						}
						nVal = SendDlgItemMessage( mhwndMaterialsDialog, IDC_LIGHTING, BM_GETCHECK, 0, 0 );
						if ( nVal == BST_CHECKED )
						{
							pxModelData->xGlobalProperties.bNoLighting = 1;
						}
						else
						{
							pxModelData->xGlobalProperties.bNoLighting = 0;
						}
						
						nVal = SendDlgItemMessage( mhwndMaterialsDialog, IDC_COLLISION_CHECK, BM_GETCHECK, 0, 0 );
						if ( nVal == BST_CHECKED )
						{
							pxModelData->xGlobalProperties.bCollisionOverride = 1;
						}
						else
						{
							pxModelData->xGlobalProperties.bCollisionOverride = 0;
						}
						GetDlgItemText( mhwndMaterialsDialog, IDC_BOUNCINESS, acString, 256 );
						fVal = (float)( strtod( acString, NULL ) );

						pxModelData->xGlobalProperties.fBounciness = fVal;

						nVal = SendDlgItemMessage( mhwndMaterialsDialog, IDC_NO_SHADOWS, BM_GETCHECK, 0, 0 );
						if ( nVal == BST_CHECKED )
						{
							pxModelData->xGlobalProperties.bNoShadows = 1;
						}
						else
						{
							pxModelData->xGlobalProperties.bNoShadows = 0;
						}
						
						nVal = SendDlgItemMessage( mhwndMaterialsDialog, IDC_LARGE_MODEL_CLIPPING, BM_GETCHECK, 0, 0 );
						if ( nVal == BST_CHECKED )
						{
							pxModelData->xGlobalProperties.bLargeModelClipping = 1;
						}
						else
						{
							pxModelData->xGlobalProperties.bLargeModelClipping = 0;
						}

						nVal = SendDlgItemMessage( mhwndMaterialsDialog, IDC_POLYGON_SPRITES_CHECK, BM_GETCHECK, 0, 0 );
						if ( nVal == BST_CHECKED )
						{
							pxModelData->xGlobalProperties.bSpriteFlags = 2;
						}
						else
						{
							nVal = SendDlgItemMessage( mhwndMaterialsDialog, IDC_SINGLE_SPRITE_CHECK, BM_GETCHECK, 0, 0 );
							if ( nVal == BST_CHECKED )
							{
								pxModelData->xGlobalProperties.bSpriteFlags = 1;
							}
							else
							{
								pxModelData->xGlobalProperties.bSpriteFlags = 0;
							}
						}

						if ( pxModelData->xVertTurretData.nModelHandle != NOTFOUND )
						{
							ModelSetGlobalProperties( pxModelData->xVertTurretData.nModelHandle, &pxModelData->xGlobalProperties );
						}
						if ( pxModelData->xHorizTurretData.nModelHandle != NOTFOUND )
						{
							ModelSetGlobalProperties( pxModelData->xHorizTurretData.nModelHandle, &pxModelData->xGlobalProperties );
						}
						if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
						{
							ModelSetGlobalProperties( pxModelData->xLowLODAttachData.nModelHandle, &pxModelData->xGlobalProperties );
						}
						if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
						{
							ModelSetGlobalProperties( pxModelData->xMedLODAttachData.nModelHandle, &pxModelData->xGlobalProperties );
						}
						if ( pxModelData->xWheel1AttachData.nModelHandle != NOTFOUND )
						{
							ModelSetGlobalProperties( pxModelData->xWheel1AttachData.nModelHandle, &pxModelData->xGlobalProperties );
						}
						if ( pxModelData->xWheel3AttachData.nModelHandle != NOTFOUND )
						{
							ModelSetGlobalProperties( pxModelData->xWheel3AttachData.nModelHandle, &pxModelData->xGlobalProperties );
						}
						if ( pxModelData->xCollisionAttachData.nModelHandle != NOTFOUND )
						{
							ModelSetGlobalProperties( pxModelData->xCollisionAttachData.nModelHandle, &pxModelData->xGlobalProperties );
						}
					}
				}
				EndDialog(hDlg, 0);
				break;
			case ID_FILE_IMPORT:
//				ModelConverterImportFile();
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

    return (FALSE);
}


void	MaterialsDialogActivate( void )
{
	DialogBox(ghInstance, (LPCTSTR)IDD_MATERIALS, NULL, (DLGPROC)ModelConverterMaterialsDlgProc );
}
