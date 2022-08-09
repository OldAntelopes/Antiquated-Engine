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
#include "../LibCode/Engine/ModelMaterialData.h"

#include "MaterialBrowser.h"
#include "ModelConverter.h"

void*	gpxMainWindow = NULL;
HWND	mhwndMaterialBrowserDialog;
int		mnMaterialBrowserCurrentAttrib = 0;


void	MaterialBrowserEnableMaterialPropertiesGroup( BOOL bFlag )
{
	if ( bFlag == TRUE )
	{
		SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_GLOBAL_CHECK, BM_SETCHECK, BST_UNCHECKED, 0 );
	}
	else
	{
		SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_GLOBAL_CHECK, BM_SETCHECK, BST_CHECKED, 0 );
	}
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT3), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT4), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT14), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT15), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT16), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT17), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT18), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT19), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT20), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT21), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT22), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT23), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT24), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT25), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT26), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT27), bFlag );
	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_EDIT28), bFlag );

	EnableWindow( GetDlgItem(mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2), bFlag );
	
}


void	MaterialBrowserInitMaterialDisplay( int nAttrib )
{
int	nModelHandle = ModelConvGetCurrentModel();
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];
ModelMaterialData*		pMaterialData;
const char*		szFilename;
MATERIAL_COLOUR	col;
char	acString[256];

	mnMaterialBrowserCurrentAttrib = nAttrib;
	pMaterialData = FindMaterial( pxModelData, nAttrib );
	if ( pMaterialData )
	{
		szFilename = pMaterialData->GetFilename(DIFFUSEMAP);
		if ( szFilename )
		{
			// TEMP - just assuming that the material is always just diffuse for now.
			SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT1, szFilename );
			SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_DIFFUSE_EMBED, BM_SETCHECK, BST_CHECKED, 0 );
			EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_REMOVE_DIFFUSE_TEXTURE ), TRUE );	
		}
		else
		{
			SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT1, "" );
			SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_DIFFUSE_EMBED, BM_SETCHECK, BST_UNCHECKED, 0 );
			EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_REMOVE_DIFFUSE_TEXTURE ), FALSE );	
		}

		szFilename = pMaterialData->GetFilename(SPECULARMAP);
		if ( szFilename )
		{
			// TEMP - just assuming that the material is always just diffuse for now.
			SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT11, szFilename );
	//		SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_DIFFUSE_EMBED, BM_SETCHECK, BST_CHECKED, 0 );
//			EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_REMOVE_DIFFUSE_TEXTURE ), TRUE );	
		}
		else
		{
			SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT11, "" );
//			SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_DIFFUSE_EMBED, BM_SETCHECK, BST_UNCHECKED, 0 );
//			EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_REMOVE_DIFFUSE_TEXTURE ), FALSE );	
		}

		szFilename = pMaterialData->GetFilename(NORMALMAP);
		if ( szFilename )
		{
			// TEMP - just assuming that the material is always just diffuse for now.
			SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT12, szFilename );
	//		SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_DIFFUSE_EMBED, BM_SETCHECK, BST_CHECKED, 0 );
//			EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_REMOVE_DIFFUSE_TEXTURE ), TRUE );	
		}
		else
		{
			SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT12, "" );
//			SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_DIFFUSE_EMBED, BM_SETCHECK, BST_UNCHECKED, 0 );
//			EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_REMOVE_DIFFUSE_TEXTURE ), FALSE );	
		}

		if ( pMaterialData->GetCloneTextureMaterialIndex(0) != NOTFOUND )
		{
			SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_CHECK14, BM_SETCHECK, BST_CHECKED, 0 );
			sprintf( acString, "Sharing diffuse texture with material %d", pMaterialData->GetCloneTextureMaterialIndex(0) + 1 );
			SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT1, acString );
		}
		else
		{
			SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_CHECK14, BM_SETCHECK, BST_UNCHECKED, 0 );
		}

		// Set blend type
		SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_SETCURSEL, (int)pMaterialData->GetBlendType(), 0 );		
		
		// Set diffuse colour
		col = pMaterialData->GetColour( ModelMaterialData::DIFFUSE );
		sprintf( acString, "%.01f", col.fRed );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT3, acString );
		sprintf( acString, "%.01f", col.fGreen );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT14, acString );
		sprintf( acString, "%.01f", col.fBlue );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT15, acString );
		sprintf( acString, "%.01f", col.fAlpha );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT16, acString );

		// Set Specular colour
		col = pMaterialData->GetColour( ModelMaterialData::SPECULAR );
		sprintf( acString, "%.01f", col.fRed );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT17, acString );
		sprintf( acString, "%.01f", col.fGreen );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT18, acString );
		sprintf( acString, "%.01f", col.fBlue );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT19, acString );
		sprintf( acString, "%.01f", col.fAlpha );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT20, acString );

		// Set Ambient colour
		col = pMaterialData->GetColour( ModelMaterialData::AMBIENT );
		sprintf( acString, "%.01f", col.fRed );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT21, acString );
		sprintf( acString, "%.01f", col.fGreen );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT22, acString );
		sprintf( acString, "%.01f", col.fBlue );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT23, acString );
		sprintf( acString, "%.01f", col.fAlpha );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT24, acString );

		// Set emissive colour
		col = pMaterialData->GetColour( ModelMaterialData::EMISSIVE );
		sprintf( acString, "%.01f", col.fRed );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT25, acString );
		sprintf( acString, "%.01f", col.fGreen );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT26, acString );
		sprintf( acString, "%.01f", col.fBlue );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT27, acString );
		sprintf( acString, "%.01f", col.fAlpha );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT28, acString );

		sprintf( acString, "%.01f", pMaterialData->GetSpecularPower() );
		SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT4, acString );

		if ( pMaterialData->HasActiveMaterialProperties() )	
		{
			MaterialBrowserEnableMaterialPropertiesGroup( TRUE );
		}
		else
		{
			MaterialBrowserEnableMaterialPropertiesGroup( FALSE );
		}
	}
	else
	{
		// No material data.. clear all bits
		MaterialBrowserEnableMaterialPropertiesGroup( FALSE );
	}

}
 

void	MaterialBrowserSelectAllPolys( void )
{
	ModelConvSetPickerMode(1);
	ModelConvSelectByMaterial(mnMaterialBrowserCurrentAttrib);
}


void	MaterialBrowserRemoveDiffuseTexture( void )
{
int	nModelHandle = ModelConvGetCurrentModel();
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];
ModelMaterialData*		pMaterialData;
int						nLoop;
int						nNumMaterials = pxModelData->xStats.nNumMaterials;

	pMaterialData = FindMaterial( pxModelData, mnMaterialBrowserCurrentAttrib );
	if ( pMaterialData )
	{
		pMaterialData->RemoveTexture( 0 );

		for ( nLoop = 0; nLoop < nNumMaterials; nLoop++ )
		{
			if ( nLoop != mnMaterialBrowserCurrentAttrib )
			{
				pMaterialData = FindMaterial( pxModelData, nLoop );
				if ( pMaterialData )
				{
					if ( pMaterialData->GetCloneTextureMaterialIndex( 0 ) == mnMaterialBrowserCurrentAttrib )
					{
						pMaterialData->CloneTextureFromMaterial( DIFFUSEMAP, NULL );
					}
				}
			}
		}

		MaterialBrowserInitMaterialDisplay( mnMaterialBrowserCurrentAttrib );
	}
}


void	MaterialBrowserChangeTexture( int tex, const char* acFilename )
{
int	nModelHandle = ModelConvGetCurrentModel();
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];
ModelMaterialData*		pMaterialData;

	pMaterialData = FindMaterial( pxModelData, mnMaterialBrowserCurrentAttrib );
	if ( pMaterialData )
	{
	FILE*	pFile;
	int		nFileSize;
	byte*	pbMem;

		pFile = fopen( acFilename, "rb" );
		if ( pFile )
		{
			nFileSize = SysGetFileSize( pFile );
			pbMem = (byte*)SystemMalloc( nFileSize );
			fread( pbMem, nFileSize,1, pFile );
			fclose( pFile );
			// In the modelconv etc, store the source data so it can be re-exported if necc
			pMaterialData->SetSourceData( tex, pbMem, nFileSize );
			SystemFree( pbMem );
			pMaterialData->LoadTextureFilename( tex, acFilename );

			switch( tex )
			{
			case 0:
			default:
				SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT1, acFilename );
				SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_DIFFUSE_EMBED, BM_SETCHECK, BST_CHECKED, 0 );
				break;
			case 1:
				SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT12, acFilename );
//				SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_DIFFUSE_EMBED, BM_SETCHECK, BST_CHECKED, 0 );
				break;
			case 2:
				SetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT11, acFilename );
//				SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_DIFFUSE_EMBED, BM_SETCHECK, BST_CHECKED, 0 );
				break;
			}
		}
	}
}



void	MaterialBrowserInitDialog( int nAttrib )
{
int	nModelHandle = ModelConvGetCurrentModel();
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];
int		loop;
char	acString[256];

	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_COMBO1, CB_RESETCONTENT, 0, 0 );

	for ( loop = 0; loop < pxModelData->xStats.nNumMaterials; loop++ )
	{
		sprintf( acString, "Material %d", loop+1 );
		SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_COMBO1, CB_ADDSTRING, loop, (LPARAM)acString );
	}
	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_COMBO1, CB_SETCURSEL, nAttrib, 0 );
	
	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_RESETCONTENT, 0, 0 );

	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_ADDSTRING, 0, (LPARAM)"None" );
	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Blend Alpha with cutoff" );
	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Additive Alpha" );
	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Subtractive Alpha" );
	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Blend Colour" );
	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Additive Colour" );
	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Subtractive Colour" );
	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_ADDSTRING, 0, (LPARAM)"Blend Alpha" );
	SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_SETCURSEL, 0, 0 );		

	MaterialBrowserInitMaterialDisplay( nAttrib );

	EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_APPLY ), FALSE );
	EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, ID_CANCEL ), FALSE );

}

/***************************************************************************
 * Function    : MaterialBrowserApplyChanges
 * Params      :
 ***************************************************************************/
void	MaterialBrowserApplyChanges( void )
{
int	nModelHandle = ModelConvGetCurrentModel();
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];
ModelMaterialData*		pMaterialData;
int		nVal;
MATERIAL_COLOUR	col;
char	acString[256];
float	fVal;

	pMaterialData = FindMaterial( pxModelData, mnMaterialBrowserCurrentAttrib );
	if ( pMaterialData )
	{
		nVal = SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_BLENDTYPE_COMBO2, CB_GETCURSEL, 0, 0 );
		pMaterialData->SetBlendType( (ModelMaterialData::eBLEND_TYPES)nVal );

		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT17, acString, 256 );
		col.fRed = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT18, acString, 256 );
		col.fGreen = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT19, acString, 256 );
		col.fBlue = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT20, acString, 256 );
		col.fAlpha = (float)atof( acString );
		// Set Specular colour
		pMaterialData->SetColour( ModelMaterialData::SPECULAR, &col );

		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT21, acString, 256 );
		col.fRed = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT22, acString, 256 );
		col.fGreen = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT23, acString, 256 );
		col.fBlue = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT24, acString, 256 );
		col.fAlpha = (float)atof( acString );
		// Set ambient  colour
		pMaterialData->SetColour( ModelMaterialData::AMBIENT, &col );

		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT25, acString, 256 );
		col.fRed = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT26, acString, 256 );
		col.fGreen = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT27, acString, 256 );
		col.fBlue = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT28, acString, 256 );
		col.fAlpha = (float)atof( acString );
		// Set emissive colour
		pMaterialData->SetColour( ModelMaterialData::EMISSIVE, &col );

		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT3, acString, 256 );
		col.fRed = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT14, acString, 256 );
		col.fGreen = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT15, acString, 256 );
		col.fBlue = (float)atof( acString );
		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT16, acString, 256 );
		col.fAlpha = (float)atof( acString );
		// Set diffuse colour
		pMaterialData->SetColour( ModelMaterialData::DIFFUSE, &col );

		GetDlgItemText( mhwndMaterialBrowserDialog, IDC_EDIT4, acString, 256 );
		fVal = (float)atof( acString );
		pMaterialData->SetSpecularPower( fVal );

		nVal = SendDlgItemMessage( mhwndMaterialBrowserDialog, IDC_GLOBAL_CHECK, BM_GETCHECK, 0, 0 );
		if ( nVal == 0 )	// if unchecked
		{
			MaterialBrowserEnableMaterialPropertiesGroup( TRUE );
			pMaterialData->SetActiveMaterialProperties( true );
		}
		else
		{
			MaterialBrowserEnableMaterialPropertiesGroup( FALSE );
			pMaterialData->SetActiveMaterialProperties( false );
		}
	}

}

void	MaterialBrowserAddNewMaterial( void )
{
int	nModelHandle = ModelConvGetCurrentModel();
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];
int		nNewMaterialAttrib;

	ModelAddNewMaterial( nModelHandle, ModelConvGetCurrentRenderTexture() );
	nNewMaterialAttrib = pxModelData->xStats.nNumMaterials - 1;
	
	MaterialBrowserInitDialog( nNewMaterialAttrib );

}

void	MaterialBrowserOnChangesMade( void )
{
	EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_APPLY ), TRUE );
//	EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, ID_CANCEL ), TRUE );
}

void	MaterialBrowserExtractTexture( int nTextureSlot )
{
char	acFilename[256];
char	acCurrentDir[256];
int	nModelHandle = ModelConvGetCurrentModel();
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];

	ModelConvGetLastLoadFolder( acCurrentDir );
	acFilename[0] = 0;

	if ( SysGetSaveFilenameDialog( "PNG File(*.png)\0*.png\0", "Extract texture file", acCurrentDir, 0, acFilename ) == TRUE )
	{
	ModelMaterialData*		pMaterialData;

		pMaterialData = FindMaterial( pxModelData, mnMaterialBrowserCurrentAttrib );
		if ( pMaterialData )
		{
		int		hTexture = pMaterialData->GetTexture(0);

			EngineExportTexture( hTexture, acFilename, 3 );
		}
	}

}

/***************************************************************************
 * Function    : MaterialBrowserDlgProc
 * Params      :
 * Returns     :
 * Description :
 ***************************************************************************/
LRESULT CALLBACK MaterialBrowserDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
int		nVal;
//LPNMHDR		pNotifyHeader;
//int		nSelected;
char	acFilename[256];
char		acCurrentDir[256];
	switch (message)
	{
	case WM_INITDIALOG:
		mhwndMaterialBrowserDialog = hDlg;
		MaterialBrowserInitDialog(0);		// todo - pass in the last material attrib we viewed
		return TRUE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case EN_CHANGE:
			MaterialBrowserOnChangesMade();
			break;
		case CBN_SELCHANGE:
			nVal = SendDlgItemMessage( hDlg, LOWORD(wParam), CB_GETCURSEL, 0, 0 );
			switch( LOWORD(wParam) )
			{
			case IDC_COMBO1:
				MaterialBrowserInitMaterialDisplay(nVal);
				break;
			case IDC_BLENDTYPE_COMBO2:
				MaterialBrowserOnChangesMade();
				break;
			}
			break;

		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDOK:
				EndDialog(hDlg, LOWORD(wParam));
				break;
			case IDC_ADDNEWMATERIAL:
				MaterialBrowserAddNewMaterial();
				break;
			case IDC_APPLY:
				MaterialBrowserApplyChanges();
				EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_APPLY ), FALSE );
				break;
			case IDC_BUTTON8:
				MaterialBrowserSelectAllPolys();
				break;
			case IDC_GLOBAL_CHECK:
				MaterialBrowserApplyChanges();
				EnableWindow( GetDlgItem( mhwndMaterialBrowserDialog, IDC_APPLY ), FALSE );
				break;
			case IDC_REMOVE_DIFFUSE_TEXTURE:
				MaterialBrowserRemoveDiffuseTexture();
				break;
			case IDC_BUTTON5:			// Change Specular map
				{
				HWND store = (HWND)SysGetMainWindow();

					SysSetMainWindow(mhwndMaterialBrowserDialog);
//					SysGetCurrentDir( 256, acCurrentDir );
					ModelConvGetLastLoadFolder( acCurrentDir );
					acFilename[0] = 0;
					if ( SysGetOpenFilenameDialog( "All supported formats (*.dds,*.bmp,*.jpg,*.tga,*.png)\0*.dds;*.jpg;*.bmp;*.tga;*.png\0DirectDraw Surface(*.dds)\0*.dds\0Jpeg File(*.jpg)\0*.jpg\0Windows bitmap(*.bmp)\0*.bmp\0All Files(*.*)\0*.*\0", "Select Specular Map", acCurrentDir, 0, acFilename ) > 0 )
					{
						MaterialBrowserChangeTexture( 2, acFilename );
					}
					SysSetMainWindow(store);
				}
				break;
			case IDC_BUTTON6:			// Change Normal map
				{
				HWND store = (HWND)SysGetMainWindow();

					SysSetMainWindow(mhwndMaterialBrowserDialog);
					ModelConvGetLastLoadFolder( acCurrentDir );
					acFilename[0] = 0;
					if ( SysGetOpenFilenameDialog( "All supported formats (*.dds,*.bmp,*.jpg,*.tga,*.png)\0*.dds;*.jpg;*.bmp;*.tga;*.png\0DirectDraw Surface(*.dds)\0*.dds\0Jpeg File(*.jpg)\0*.jpg\0Windows bitmap(*.bmp)\0*.bmp\0All Files(*.*)\0*.*\0", "Select Normal Map", acCurrentDir, 0, acFilename ) > 0 )
					{
						MaterialBrowserChangeTexture( 1, acFilename );
					}
					SysSetMainWindow(store);
				}
				break;
			case IDC_EXTRACT_DIFFUSE_TEX:
				MaterialBrowserExtractTexture( 0 );
				break;
			case IDC_CHANGE_DIFFUSE:
				{
				HWND store = (HWND)SysGetMainWindow();

					SysSetMainWindow(mhwndMaterialBrowserDialog);
					ModelConvGetLastLoadFolder( acCurrentDir );
					acFilename[0] = 0;
					if ( SysGetOpenFilenameDialog( "All supported formats (*.dds,*.bmp,*.jpg,*.tga,*.png)\0*.dds;*.jpg;*.bmp;*.tga;*.png\0DirectDraw Surface(*.dds)\0*.dds\0Jpeg File(*.jpg)\0*.jpg\0Windows bitmap(*.bmp)\0*.bmp\0All Files(*.*)\0*.*\0", "Select Diffuse Texture", acCurrentDir, 0, acFilename ) > 0 )
					{
						// TODO - Cope with multiple textures in material
						MaterialBrowserChangeTexture( 0, acFilename );
					}
					SysSetMainWindow(store);
				}
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
		EndDialog(hDlg, LOWORD(wParam));
		return( 0 );
	case WM_DESTROY:
		return(0);
	default:
		break;
	}

	return( FALSE );
}

void	MaterialBrowserDeleteAllMaterials( void )
{
	ModelDeleteAllMaterials( ModelConvGetCurrentModel() );
}

void	MaterialBrowserAddDefaultMaterials( void )
{
const char*	pcTexture = ModelConvGetCurrentRenderTexture();

	ModelCreateDefaultMaterials( ModelConvGetCurrentModel(), pcTexture );
}


