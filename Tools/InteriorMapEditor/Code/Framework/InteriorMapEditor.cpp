//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
//#define	WINVER	0x0500

#include <stdwininclude.h>
//#include <afxcmn.h>

#include <StandardDef.h>

//#include <windows.h>
#include <CommCtrl.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>
// Temp - get rid of this when engine has its references to the d3d device internalised

#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>
#include <Rendering.h>
#include <System.h>
#include "../Util/MouseCam.h"
#include "../Util/Line Renderer.h"
#include "../Util/HttpDownload.h"
#include "../../Resource.h"
#include "../../LibCode/Engine/Scene/SceneMap.h"		// cheeky
#include "../Main.h"
#include "InteriorMapEditor.h"

HINSTANCE	ghInstance;
HWND		ghwndMain;
char*	mszProjectClassName = "InteriorMapEditor";
BOOL	mboMinimised = FALSE;
BOOL	mboQuitSelected = FALSE;

extern "C"
{
extern void*	gpxMainWindow;
extern int		gnDenariPerSheckle;
}
void*	gpxMainWindow = NULL;
int		gnDenariPerSheckle = 10;
char	mszDefaultFilePath[256] = "";

HWND	mhwndMainDialog;
HWND	mhwndGraphicWindow;
BOOL	mboMapEdRightMouseDown = FALSE;
VECT	mxMapEdMouseDownPos;
BOOL	mbXAxisActive = TRUE;
BOOL	mbYAxisActive = TRUE;
BOOL	mbZAxisActive = TRUE;

HBITMAP 	mhbmpMoveSel;
HBITMAP 	mhbmpRotSel;
HBITMAP 	mhbmpZoomSel;
HBITMAP 	mhbmpMove;
HBITMAP 	mhbmpRot;
HBITMAP 	mhbmpZoom;

#define		MAX_ITEMS_SELECTED		128

typedef struct
{
	BOOL	mbViewRooms;
	BOOL	mbViewFurniture;
	BOOL	mbViewSpawnExitPoints;
	BOOL	mbViewScriptEvents;
	BOOL	mbViewBuildingAccess;
	BOOL	mbViewInteriorLinks;

} VIEW_OPTIONS;

VIEW_OPTIONS		mxViewOptions = { TRUE, TRUE, TRUE, TRUE, TRUE, TRUE };

typedef struct
{
	SceneMapElement*		pElement;
	VECT					xOrigPos;
	VECT					xOrigRot;
} MAPED_SELECTION;

typedef struct
{
	SceneMapElement*		pElement;
	VECT					xOrigPos;
	VECT					xOrigRot;

} UNDO_BUFFER;

UNDO_BUFFER			mxUndoBuffer;

MAPED_SELECTION		maSelectedItems[ MAX_ITEMS_SELECTED ];
int					mnNumItemsInSelection = 0;

/*
class	CMultiTreeCtrl : public CTreeCtrl 
{
	

};
*/

void	MapEditorInitTree( void )
{


}


void UpdateControlIcons( void )
{

	SendDlgItemMessage(mhwndMainDialog, IDC_MOVE_BUT, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HANDLE)mhbmpMove );
	SendDlgItemMessage(mhwndMainDialog, IDC_ROT_BUT, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HANDLE)mhbmpRot );
	SendDlgItemMessage(mhwndMainDialog, IDC_ZOOM_BUT, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HANDLE)mhbmpZoom );

	switch( MouseCamGetControlMode() )
	{
	case CONTROL_MOVE:
		SendDlgItemMessage(mhwndMainDialog, IDC_MOVE_BUT, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HANDLE)mhbmpMoveSel );
		UpdateWindow( GetDlgItem( mhwndMainDialog, IDC_MOVE_BUT ) );
		break;
	case CONTROL_ROT:
		SendDlgItemMessage(mhwndMainDialog, IDC_ROT_BUT, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HANDLE)mhbmpRotSel );
		UpdateWindow( GetDlgItem( mhwndMainDialog, IDC_ROT_BUT ) );
		break;
	case CONTROL_ZOOM:
		SendDlgItemMessage(mhwndMainDialog, IDC_ZOOM_BUT, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HANDLE)mhbmpZoomSel );
		UpdateWindow( GetDlgItem( mhwndMainDialog, IDC_ZOOM_BUT ) );
		break;
	}
}

void		MapEditorSelectNewElement( SceneMapElement* pSelected )
{
	mnNumItemsInSelection = 0;
	if ( maSelectedItems[ 0 ].pElement )
	{
		maSelectedItems[ 0 ].pElement->SetSelected( FALSE );
		maSelectedItems[ 0 ].pElement = 0;
	}

	if ( pSelected )
	{
	char	szString[1024];

		pSelected->SetSelected( TRUE );
		maSelectedItems[ 0 ].pElement = pSelected;
		mnNumItemsInSelection = 1;

		if ( stricmp( pSelected->GetTypeName(), "Region" ) == 0 )
		{
		SceneRegionElement*	pRegion = (SceneRegionElement*)( pSelected );
			if ( pRegion->GetRegionType() )
			{
				sprintf(szString,"Selected item:\r\nRegion - Type: %s\r\n", pRegion->GetRegionType() );
			}
			else
			{
				sprintf(szString,"Selected item:\r\nRegion - Type: None\r\n", pRegion->GetRegionType() );
			}
			SetDlgItemText(mhwndMainDialog, IDC_DETAILS_TEXT, szString );

			if ( pRegion->GetRegionParam() )
			{
				SetDlgItemText( mhwndMainDialog, IDC_PARAM_EDIT, pRegion->GetRegionParam() );
			}
			else
			{
				SetDlgItemText( mhwndMainDialog, IDC_PARAM_EDIT, "" );
			}
			int		nVal = (int)(pRegion->GetScale() * 1000);
			SendDlgItemMessage( mhwndMainDialog, IDC_SLIDER1, TBM_SETPOS, TRUE, (LPARAM)nVal); 

			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_RADIUS_TEXT ), TRUE );
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_PARAM_EDIT ), TRUE );
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_SLIDER1 ), TRUE );
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_TOOLBOX ), TRUE );
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_PARAMLABEL ), TRUE );
		}
		else
		{
			sprintf(szString,"Selected item:\r\nModel: %s\r\nTexture: %s\r\n", pSelected->GetModelFilename(), pSelected->GetTextureFilename() );
			if ( !pSelected->IsModelLoaded() )
			{
				sprintf( szString, "%sModel not loaded!\r\n", szString );
			}
			if ( !pSelected->IsTextureLoaded() )
			{
				sprintf( szString, "%sTexture not loaded!\r\n", szString );
			}
			SetDlgItemText(mhwndMainDialog, IDC_DETAILS_TEXT, szString );

			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_RADIUS_TEXT ), FALSE );
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_PARAM_EDIT ), FALSE );
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_SLIDER1 ), FALSE );
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_TOOLBOX ), FALSE );
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_PARAMLABEL ), FALSE );
		}
	}
}

void	InteriorMapEditorChangeView( int nSelected )
{
	switch( nSelected )
	{
	case 0:
	default:
		MouseCamSetViewMode( VIEW_MODE_NORMAL );
		break;
	case 1:
		MouseCamSetViewMode( VIEW_MODE_TOPDOWN );
		break;
	}
}

void	MapEdUndo( void )
{
int		Loop;
	for ( Loop = 0; Loop < mnNumItemsInSelection; Loop++ )
	{
		maSelectedItems[ Loop ].pElement->SetPos( &maSelectedItems[ Loop ].xOrigPos );
		maSelectedItems[ Loop ].pElement->SetRot( &maSelectedItems[ Loop ].xOrigRot );
	}
}

void MapEdRightMouseDown( float fX, float fY )
{
int		Loop;

	mxMapEdMouseDownPos.x = fX;
	mxMapEdMouseDownPos.y = fY;
	mboMapEdRightMouseDown = TRUE;

	for ( Loop = 0; Loop < mnNumItemsInSelection; Loop++ )
	{
		maSelectedItems[ Loop ].xOrigPos = *maSelectedItems[ Loop ].pElement->GetPos();
		maSelectedItems[ Loop ].xOrigRot = *maSelectedItems[ Loop ].pElement->GetRot();
	}
}

void MapEdRightMouseDownMove( float fXDelta, float fYDelta )
{
VECT	xMove;
int		Loop;
VECT	xNew;

	fXDelta = fXDelta - mxMapEdMouseDownPos.x;
	fYDelta = fYDelta - mxMapEdMouseDownPos.y;
	switch( MouseCamGetControlMode() )
	{
	case CONTROL_MOVE:
	case CONTROL_ZOOM:
	default:
		xMove = MouseCamGetMoveFromMouseDelta( fXDelta, fYDelta );
		for ( Loop = 0; Loop < mnNumItemsInSelection; Loop++ )
		{
			xNew = maSelectedItems[ Loop ].xOrigPos;
			if ( mbXAxisActive )
			{
				xNew.x -= xMove.x;
			}
			if ( mbYAxisActive )
			{
				xNew.y -= xMove.y;
			}
			if ( mbZAxisActive )
			{
				xNew.z -= xMove.z;
			}
			maSelectedItems[ Loop ].pElement->SetPos( &xNew );
		}
		break;
	case CONTROL_ROT:
		break;
	}
}

//--------------------------------------------------------------------
// WindowsMsgProc
//    The window's message handler
//--------------------------------------------------------------------
LRESULT WINAPI GraphicWindowMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
POINTS points;
    switch( msg )
    {
	case WM_INITDIALOG:
		return TRUE;
	case WM_KEYDOWN:
		//Handle a key press event
		switch (wParam)
		{
		case VK_RETURN:
			TreeView_EndEditLabelNow(GetDlgItem(mhwndMainDialog, IDC_TREE1),FALSE);
			break;
		case VK_ESCAPE:
			PostQuitMessage( 0 );
			return( DefWindowProc( hWnd, msg, wParam, lParam ) );
			break;
		}
		break;
	case WM_CHAR:
		switch ( (short)( wParam ) )
		{
		case 'Z':
		case 'z':
			MouseCamSetControlMode( CONTROL_ZOOM );
			UpdateControlIcons();
			break;
		case 'X':
		case 'x':
			MouseCamSetControlMode( CONTROL_MOVE );
			UpdateControlIcons();
			break;
		case 'C':
		case 'c':
			MouseCamSetControlMode( CONTROL_ROT );
			UpdateControlIcons();
			break;
/*		case 'V':
		case 'v':
			ModelConvResetCamera();
			ModelConverterSetupCamera();
			ModelConverterDisplayFrame();
			break;
*/		}
		break;
	case WM_SIZE:
		break;
	case WM_MOVE:
		break;
	case WM_RBUTTONDOWN:
		points = MAKEPOINTS(lParam);
		MapEdRightMouseDown( points.x, points.y );
		break;    
	case WM_RBUTTONUP:
		mboMapEdRightMouseDown = FALSE;
		break;    
	case WM_MOUSEMOVE:
		if ( mboMapEdRightMouseDown == TRUE )
		{
			points = MAKEPOINTS(lParam);
			MapEdRightMouseDownMove( points.x, points.y);
		}
		else
		{
			MouseCamWindowMsgProc( hWnd, msg, wParam,lParam );
		}
		break;
	case WM_MBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		MouseCamWindowMsgProc( hWnd, msg, wParam,lParam );
		break;
	case WM_DESTROY:
		// Handle close & quit
		PostQuitMessage( 0 );
		return 0;
		break;
 	case WM_CLOSE:
		mboQuitSelected = TRUE;
		break;
	default:
		break;
	}

    return( DefWindowProc( hWnd, msg, wParam, lParam ) );
}


void MapEditorInitGraphicWindow( void)
{
RECT		xWindowRect;
WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, GraphicWindowMsgProc, 0L, 0L,GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "MapEd", NULL };
WNDCLASSEX*	pWinClass = &wc;
DWORD		dwWindowStyle;
HWND	hwndParent;
	RegisterClassEx( pWinClass );

	hwndParent = GetDlgItem(mhwndMainDialog, IDC_TAB1 );
	RECT rect;
	GetClientRect(hwndParent, &rect);

	dwWindowStyle = WS_CHILD;
	xWindowRect.left = rect.left + 4;
	xWindowRect.right = rect.right - 5;
	xWindowRect.bottom = rect.bottom - 28;
	xWindowRect.top = rect.top + 4;
	AdjustWindowRect( &xWindowRect, dwWindowStyle, FALSE );
	InterfaceSetInitialSize( FALSE, xWindowRect.right - xWindowRect.left, xWindowRect.bottom - xWindowRect.top, FALSE );
    // Create the application's window
     mhwndGraphicWindow = CreateWindow( "MapEd", "MapEditor",
                              dwWindowStyle, xWindowRect.left, xWindowRect.top,
						      xWindowRect.right - xWindowRect.left, xWindowRect.bottom - xWindowRect.top,
                              hwndParent, NULL, ghInstance, NULL );
	InterfaceSetWindow( mhwndGraphicWindow );
	InterfaceInitDisplayDevice( TRUE );
//	InterfaceInitDisplayDevice(TRUE);
	EngineInitFromInterface();

	ShowWindow( mhwndGraphicWindow, SW_SHOW );
	UpdateWindow( mhwndGraphicWindow );
	InterfaceInit( TRUE );
	InterfaceNewFrame( 0 );
	InterfacePresent();

	InitialiseLineRenderer(32768);
	ModelRenderingInit();
}

HTREEITEM		mhTreeRoot;
HTREEITEM		mhCurrentLeaf;

void	InteriorMapEditorNewFrame( void )
{
uint32		ulCol = 0x80808080;

	MouseCamSetMoveSpeed(4.0f);
	MouseCamSetRotSpeed(1.2f);
	MouseCamSetZoomSpeed(3.0f);
	// If we need to change and we currently have a d3d device
	if ( InterfaceDoesNeedChanging() == TRUE ) 
	{
		// Free all graphics
//		GameFreeAllGraphics();
		FreeLineRenderer();
//		mboIsResizing = TRUE;
	}
	switch(	InterfaceNewFrame( ulCol ) )
	{
	case 0:
		break;
	case 1:
		InterfaceInit( TRUE );
		InitialiseLineRenderer(32768);
		InteriorMapEditorInitScene();
//		mboIsResizing = FALSE;
		break;
	default:
		PostQuitMessage(0);
		break;
	}
}

HTREEITEM mhSelectedTreeItem;

void	MapEditorTreeViewAddElements( SceneMapElement* pElement )
{
TVINSERTSTRUCT	xInsertStruct;
char	acString[256];
SceneMapElement*	pChild;
HTREEITEM	hParent = mhCurrentLeaf;

	while ( pElement )
	{
		sprintf( acString, pElement->GetTypeName() );
		if ( stricmp( acString, "Region" ) == 0 )
		{
		SceneRegionElement*		pRegion = (SceneRegionElement*)pElement;
			if ( pRegion->GetRegionType() )
			{
				sprintf( acString, pRegion->GetRegionType() );
			}
		}
		xInsertStruct.hParent = mhCurrentLeaf;
		xInsertStruct.hInsertAfter = TVI_SORT;
		xInsertStruct.item.mask = TVIF_TEXT | TVIF_PARAM;//| TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		xInsertStruct.item.pszText = acString;
		xInsertStruct.item.iImage = 0;//;
		xInsertStruct.item.iSelectedImage = 0;//mnOpenImage;
		xInsertStruct.item.lParam = (LPARAM)( pElement );

		mhCurrentLeaf = (HTREEITEM)( SendDlgItemMessage( mhwndMainDialog, IDC_TREE1, TVM_INSERTITEM, 0, (LPARAM)(&xInsertStruct) ) );

		if ( maSelectedItems[ 0 ].pElement == pElement )
		{
			mhSelectedTreeItem = mhCurrentLeaf;
		}
		pChild = pElement->GetChild();
		if ( pChild )
		{
			MapEditorTreeViewAddElements( pChild );
		}
		mhCurrentLeaf = hParent;
		pElement = pElement->GetBrother();
	}
}

void	MapEditorRefreshTreeview( void )
{
TVINSERTSTRUCT	xInsertStruct;
char	acString[256];
SceneMapElement*	pElement;

	TreeView_DeleteAllItems( GetDlgItem( mhwndMainDialog, IDC_TREE1 ) );
	sprintf( acString, "SceneRoot" );
	xInsertStruct.hParent = TVI_ROOT;
	xInsertStruct.hInsertAfter = TVI_SORT;
	xInsertStruct.item.mask = TVIF_TEXT;//| TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	xInsertStruct.item.pszText = acString;
	xInsertStruct.item.iImage = 0;//;
	xInsertStruct.item.iSelectedImage = 0;//mnOpenImage;

	mhTreeRoot = (HTREEITEM)( SendDlgItemMessage( mhwndMainDialog, IDC_TREE1, TVM_INSERTITEM, 0, (LPARAM)(&xInsertStruct) ) );

	pElement = SceneMapGetBaseElement();
	mhCurrentLeaf = mhTreeRoot;
	MapEditorTreeViewAddElements( pElement );

	if ( maSelectedItems[ 0 ].pElement )
	{
		TreeView_SelectItem( GetDlgItem( mhwndMainDialog, IDC_TREE1 ), mhSelectedTreeItem );
	}
	else
	{
		TreeView_SelectItem( GetDlgItem( mhwndMainDialog, IDC_TREE1 ), NULL );
	}

}


void	MapEditorInitDialog( void )
{
TCITEM	xTCItem;
char	acString[256];

	MapEditorInitTree();
	sprintf( acString, "3d View" );
	xTCItem.mask = TCIF_TEXT;
	xTCItem.pszText = acString;
	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_INSERTITEM, 0, (LPARAM)( &xTCItem ) );

	sprintf( acString, "Top" );
	xTCItem.mask = TCIF_TEXT;
	xTCItem.pszText = acString;
	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_INSERTITEM, 1, (LPARAM)( &xTCItem ) );

	sprintf( acString, "Player View" );
	xTCItem.mask = TCIF_TEXT;
	xTCItem.pszText = acString;
	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_INSERTITEM, 2, (LPARAM)( &xTCItem ) );

	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_SETCURSEL , 0, 0 );

	SendDlgItemMessage( mhwndMainDialog,IDC_CHECK1, BM_SETCHECK, BST_CHECKED, 0 );
	SendDlgItemMessage( mhwndMainDialog,IDC_CHECK2, BM_SETCHECK, BST_CHECKED, 0 );
	SendDlgItemMessage( mhwndMainDialog,IDC_CHECK3, BM_SETCHECK, BST_CHECKED, 0 );

	// Init CherryTree control

}

char*	mszLastFolderStoreFile = "C:\\UniSceneEd.tmp";

void	InteriorMapEditorStoreDefaultFilepath( char* acFileName )
{
int		nLoop;
char	acFilePath[256];

	nLoop = strlen( acFileName );
	while ( ( nLoop > 0 ) &&
			( acFileName[nLoop] != '\\' ) &&
			( acFileName[nLoop] != '/' ) )
	{
		nLoop--;
	}
	strcpy( acFilePath, acFileName );
	acFilePath[nLoop] = 0;
	strcpy( mszDefaultFilePath, acFilePath );
	
	FILE*	pFile = fopen( mszLastFolderStoreFile, "wb" );
	if ( pFile )
	{
		fwrite( mszDefaultFilePath, strlen( mszDefaultFilePath ) + 1, 1, pFile );
		fclose( pFile );
	}
}

void	InteriorMapEditorInitDefaultFilepath( void )
{
FILE*	pFile = fopen( mszLastFolderStoreFile, "rb" );
int		nFileSize;
	if ( pFile )
	{
		nFileSize = SysGetFileSize( pFile );
		fread( mszDefaultFilePath, nFileSize, 1, pFile );
		fclose( pFile );
	}
}

void InteriorMapEditorAddNewRoom( void )
{
char	acModelFilename[256];
char	acTextureFilename[256];
SceneMapElement*		pSelected;

	acModelFilename[0] = 0;
	acTextureFilename[0] = 0;

	if ( SysGetOpenFilenameDialog( "Model File(*.atm)\0*.atm\0All Files(*.*)\0*.*\0", "Select Room Model", mszDefaultFilePath, 0, acModelFilename ) > 0 )
	{
		InteriorMapEditorStoreDefaultFilepath(acModelFilename);
		if ( SysGetOpenFilenameDialog( "Texture File(*.jpg)\0*.jpg\0All Files(*.*)\0*.*\0", "Select Room Texture", mszDefaultFilePath, 0, acTextureFilename ) > 0 )
		{
			pSelected = SceneMapAddElement( SceneMapElement::ROOM, acModelFilename, acTextureFilename, NULL );
			MapEditorSelectNewElement( pSelected );
			MapEditorRefreshTreeview();
		}
		else
		{
			pSelected = SceneMapAddElement( SceneMapElement::ROOM, acModelFilename, NULL, NULL );
			MapEditorSelectNewElement( pSelected );
			MapEditorRefreshTreeview();
		}
	}
}


void InteriorMapEditorAddNewFurniture( void )
{
char	acModelFilename[256];
char	acTextureFilename[256];
	
	if ( mnNumItemsInSelection < 1 )
	{
		SysMessageBox( "A room must be selected to add the furniture to", "Unable to add furniture", SYSMESSBOX_EXCLAMATION | SYSMESSBOX_OK );
		return;
	}
	acModelFilename[0] = 0;
	acTextureFilename[0] = 0;

	if ( SysGetOpenFilenameDialog( "Model File(*.atm)\0*.atm\0All Files(*.*)\0*.*\0", "Select Furniture Model", mszDefaultFilePath, 0, acModelFilename ) > 0 )
	{
		InteriorMapEditorStoreDefaultFilepath(acModelFilename);

		if ( SysGetOpenFilenameDialog( "Texture File(*.jpg)\0*.jpg\0All Files(*.*)\0*.*\0", "Select Furniture Texture", mszDefaultFilePath, 0, acTextureFilename ) > 0 )
		{
		SceneMapElement*		pSelected;
		SceneMapElement*		pParent = maSelectedItems[ 0 ].pElement;

			// If we have an item of furniture selected..
			if ( stricmp( maSelectedItems[ 0 ].pElement->GetTypeName(), "Furniture" ) == 0 )
			{
				// Use the selected furniture's parent as our own
				pParent = SceneMapGet()->FindParentOf(maSelectedItems[ 0 ].pElement);
			}
			pSelected = SceneMapAddElement( SceneMapElement::FURNITURE, acModelFilename, acTextureFilename, pParent );
			MapEditorSelectNewElement( pSelected );
			MapEditorRefreshTreeview();
		}
	}
}

void InteriorMapEditorAddNewRegion( const char* szRegionType, const char* szRegionParam )
{
SceneMapElement*	pNewElement = SceneMapAddElement( SceneMapElement::REGION, NULL, NULL, maSelectedItems[ 0 ].pElement );
	if ( pNewElement )
	{
	SceneRegionElement* pRegion = (SceneRegionElement*)(pNewElement);
		pRegion->SetRegionType( szRegionType );
		pRegion->SetRegionParam( szRegionParam );
		MapEditorSelectNewElement( pNewElement );
	}
	MapEditorRefreshTreeview();
}

void	InteriorMapEditorSetSelectionRotation( float fRot )
{
	maSelectedItems[ 0 ].pElement->SetYaw( fRot );
}




LRESULT CALLBACK EditPropertiesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;

	switch (message)
	{
		case WM_INITDIALOG:
			SendDlgItemMessage( hDlg, IDC_COMBO1, CB_RESETCONTENT, 0, 0 );
			SendDlgItemMessage( hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Free roam" );
			SendDlgItemMessage( hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Fixed building" );
			SendDlgItemMessage( hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Camera spline" );
			SendDlgItemMessage( hDlg, IDC_COMBO1, CB_SETCURSEL, 0, 0 );
			return TRUE;

		case WM_COMMAND:
			wNotifyCode = HIWORD(wParam); 
			switch ( wNotifyCode )
			{
			case LBN_SELCHANGE:
				break;
			case LBN_DBLCLK:
				break;
			default:		
				switch( LOWORD(wParam) )
				{
				case IDOK:
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

int	mnNumLinesTextAdded = 0;

void	DemoFrameworkDebugPrint( const char* szText )
{
	SendDlgItemMessage( mhwndMainDialog, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)( szText ) );
	SendDlgItemMessage( mhwndMainDialog, IDC_LIST1, LB_SETTOPINDEX, (WPARAM)(mnNumLinesTextAdded), 0 );
	mnNumLinesTextAdded++;
}


void	MapEditorResizeWindow( int nWidth, int nHeight )
{
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_TAB1 ), NULL, 0, 0, nWidth-175, nHeight-160, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOMOVE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_LIST1 ), NULL, 560, nHeight - 120, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_LIST1 ), NULL, 560, nHeight - 120, nWidth-565, 115, SWP_SHOWWINDOW | SWP_NOZORDER);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_DETAILS_TEXT ), NULL, 20, nHeight - 110, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_DETAILS_BORDER ), NULL, 10, nHeight - 130, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	
	SetWindowPos( mhwndGraphicWindow, NULL, 0, 0, nWidth-190, nHeight-195, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOMOVE);
	InterfaceSetWindowSize( FALSE, nWidth-190, nHeight-190, TRUE );
	
}

void	InteriorMapEditorToolSliderChange( float fVol )
{
SceneMapElement* pSelected = maSelectedItems[0].pElement;

	if ( pSelected )
	{
		if ( stricmp( pSelected->GetTypeName(), "Region" ) == 0 ) 
		{
		SceneRegionElement* pRegion = (SceneRegionElement*)pSelected;

			pRegion->SetScale( fVol );
		}
	}

}



void		MapEditorCentreOnSelected( void )
{
SceneMapElement* pSelected = maSelectedItems[0].pElement;

	if ( pSelected )
	{
	VECT	xCamPos;
	VECT	xCamDir;

		xCamDir = *EngineCameraGetDirection();
		xCamPos = *pSelected->GetPos();
		VectNormalize( &xCamDir );
		VectScale( &xCamDir, &xCamDir, -1.0f );
		VectAdd( &xCamPos, &xCamPos, &xCamDir );
		xCamDir = *EngineCameraGetDirection();
		MouseCamSet( &xCamPos, pSelected->GetPos() );
		MouseCamUpdateCameraMatrices();
//		MouseCamUpdate();
	}

}



/***************************************************************************
 * Function    : MapEditorMainDlgProc
 * Params      :
 * Description : Message handler
 ***************************************************************************/
LRESULT CALLBACK MapEditorMainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
LPNMHDR		pNotifyHeader;
int	nVal;
char	acFilename[256];
SceneMapElement* pSelected;
float	fVol = 1.0f;
int		nRange;

	switch (message)
	{
	case WM_TIMER:
		InteriorMapEditorUpdateFrame();
		SetTimer( mhwndMainDialog, 1, 20, NULL );
		break;
	case WM_NOTIFY:
		pNotifyHeader = (LPNMHDR) lParam; 
		wNotifyCode = pNotifyHeader->code;
		switch ( wNotifyCode )
		{
		case TCN_SELCHANGE:
			nVal = SendDlgItemMessage( hDlg, IDC_TAB1, TCM_GETCURSEL , 0, 0 );
			InteriorMapEditorChangeView( nVal );
			break;
		case TVN_BEGINLABELEDIT:
			return( false );
			
		case TVN_ENDLABELEDIT:
			{
			TV_DISPINFO* ptvdi = (TV_DISPINFO FAR *) lParam;
				if ( ptvdi->item.pszText )
				{
					pSelected = (SceneMapElement*)ptvdi->item.lParam;
					if ( pSelected )
					{
					char	acString[256];
						strcpy( acString, ptvdi->item.pszText );
						ptvdi->item.mask |= TVIF_TEXT;
						ptvdi->item.pszText = acString;

						return( TRUE );
					}
				}
				return( FALSE );
			}
			break;
		case TVN_SELCHANGING:
			{
			LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) lParam;
				if ( pnmtv->action == TVC_BYMOUSE )
				{
					// if no shift
					if ( GetKeyState( VK_SHIFT) >= 0 )
					{
						pSelected = (SceneMapElement*)pnmtv->itemNew.lParam;
						SceneMapElement* pOld = (SceneMapElement*)pnmtv->itemOld.lParam;

						MapEditorSelectNewElement( pSelected );
						return( FALSE );
					}
					else	// If shift held, add to multiple selection
					{
						mnNumItemsInSelection = 0;
						return( TRUE );
					}
				}
			}
			break;
/*		case TCN_SELCHANGE:
			mnViewPage = SendDlgItemMessage( hDlg, IDC_TAB1, TCM_GETCURSEL , 0, 0 );
			ModelConverterDisplayFrame();
			break;
*/		}
		break;
	case WM_INITDIALOG:
		mhwndMainDialog = hDlg;
		gpxMainWindow = hDlg;
		MapEditorInitDialog();
		MapEditorInitGraphicWindow();
		InteriorMapEditorInit();
		MapEditorRefreshTreeview();
		SetTimer( mhwndMainDialog, 1, 50, NULL );
		SendDlgItemMessage(hDlg, IDC_ROT_BUT, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HANDLE)mhbmpRotSel );

		SendDlgItemMessage( hDlg, IDC_SLIDER1, TBM_SETRANGE, FALSE, (LPARAM)MAKELONG(0,10000)); 
		SendDlgItemMessage( hDlg, IDC_SLIDER1, TBM_SETPOS, TRUE, (LPARAM)1000); 
		return TRUE;
	case WM_HSCROLL:
		switch ( LOWORD(wParam) )
		{
		case TB_THUMBPOSITION:
		case TB_THUMBTRACK:
			fVol = (float)(HIWORD(wParam)) * 0.001f;
			break;
		default:
			if ( (HWND)lParam == GetDlgItem( hDlg,IDC_SLIDER1 ) )
			{
				nRange = SendDlgItemMessage( hDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0 );
			}
			fVol = nRange * 0.001f;
			break;
		}
		InteriorMapEditorToolSliderChange( fVol );
		break;
	case WM_KEYDOWN:
		switch( wParam )
		{
		case VK_RETURN:
			TreeView_EndEditLabelNow(GetDlgItem(hDlg, IDC_TREE1),FALSE);
			break;
		case VK_ESCAPE:		
			TreeView_EndEditLabelNow(GetDlgItem(hDlg, IDC_TREE1),TRUE);
			break;
	      }
		break;
	case WM_CHAR:
		switch ( (short)( wParam ) )
		{
		case 'Z':
		case 'z':
			MouseCamSetControlMode( CONTROL_ZOOM );
			UpdateControlIcons();
			break;
		case 'X':
		case 'x':
			MouseCamSetControlMode( CONTROL_MOVE );
			UpdateControlIcons();
			break;
		case 'C':
		case 'c':
			MouseCamSetControlMode( CONTROL_ROT );
			UpdateControlIcons();
			break;
/*		case 'V':
		case 'v':
			ModelConvResetCamera();
			ModelConverterSetupCamera();
			ModelConverterDisplayFrame();
			break;
*/		}
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case CBN_SELCHANGE:
			break;

			case EN_CHANGE:
				switch( LOWORD(wParam) )
				{
				case IDC_PARAM_EDIT:
					char	acString[256];
					GetDlgItemText( hDlg, LOWORD(wParam), acString, 256);
					SceneRegionElement*	pRegion = (SceneRegionElement*)maSelectedItems[0].pElement;
					if ( ( pRegion ) &&
						 ( stricmp( pRegion->GetTypeName(), "Region" ) == 0 ) )
					{
						pRegion->SetRegionParam( acString );
					}
					break;
				}
				break;
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case ID_EDIT_CHANGEMODEL:
				{
				char	acModelName[256];
					acModelName[0] = 0;
					if ( SysGetOpenFilenameDialog( "Model File(*.atm)\0*.atm\0All Files(*.*)\0*.*\0", "Select Model", mszDefaultFilePath, 0, acModelName ) > 0 )
					{
						InteriorMapEditorStoreDefaultFilepath(acModelName);
						maSelectedItems[ 0 ].pElement->SetModel( acModelName );
					}
				}
				break;
			case ID_EDIT_CHANGETEXTURE:
				{
				char	acTextureName[256];
					acTextureName[0] = 0;
					if ( SysGetOpenFilenameDialog( "Texture File(*.jpg)\0*.jpg\0All Files(*.*)\0*.*\0", "Select Texture", mszDefaultFilePath, 0, acTextureName ) > 0 )
					{
						InteriorMapEditorStoreDefaultFilepath(acTextureName);
						maSelectedItems[ 0 ].pElement->SetTexture( acTextureName );
					}
				}
				break;
			case ID_EDIT_SETROTATION_0:
				InteriorMapEditorSetSelectionRotation( 0 );
				break;
			case ID_EDIT_ROTATE_90:
				InteriorMapEditorSetSelectionRotation( 90 );
				break;
			case ID_EDIT_SETROTATION_180:
				InteriorMapEditorSetSelectionRotation( 180 );
				break;
			case ID_EDIT_SETROTATION_270:
				InteriorMapEditorSetSelectionRotation( 270 );
				break;
			case ID_VIEW_SHOWROOMS:
				mxViewOptions.mbViewRooms = !mxViewOptions.mbViewRooms;
				if ( mxViewOptions.mbViewRooms )
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWROOMS, MF_UNCHECKED);
				}
				else
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWROOMS, MF_CHECKED);
				}
				break;
			case ID_VIEW_SHOWFURNITURE:
				mxViewOptions.mbViewFurniture = !mxViewOptions.mbViewFurniture;
				if ( mxViewOptions.mbViewFurniture )
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWFURNITURE, MF_UNCHECKED);
				}
				else
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWFURNITURE, MF_CHECKED);
				}
				break;
			case ID_VIEW_SHOWSPAWNPOINTS:
				mxViewOptions.mbViewSpawnExitPoints = !mxViewOptions.mbViewSpawnExitPoints;
				if ( mxViewOptions.mbViewSpawnExitPoints )
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWSPAWNPOINTS, MF_UNCHECKED);
				}
				else
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWSPAWNPOINTS, MF_CHECKED);
				}
				break;
			case ID_VIEW_SHOWSCRIPTREGIONS:
				mxViewOptions.mbViewScriptEvents = !mxViewOptions.mbViewScriptEvents;
				if ( mxViewOptions.mbViewScriptEvents )
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWSCRIPTREGIONS, MF_UNCHECKED);
				}
				else
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWSCRIPTREGIONS, MF_CHECKED);
				}
				break;
			case ID_VIEW_SHOWBUILDINGACCESSPOINTS:
				mxViewOptions.mbViewBuildingAccess = !mxViewOptions.mbViewBuildingAccess;
				if ( mxViewOptions.mbViewBuildingAccess )
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWBUILDINGACCESSPOINTS, MF_UNCHECKED);
				}
				else
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWBUILDINGACCESSPOINTS, MF_CHECKED);
				}
				break;
			case ID_VIEW_SHOWINTERIORLINKS:
				mxViewOptions.mbViewInteriorLinks = !mxViewOptions.mbViewInteriorLinks;
				if ( mxViewOptions.mbViewInteriorLinks )
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWINTERIORLINKS, MF_UNCHECKED);
				}
				else
				{
					CheckMenuItem(GetMenu(hDlg), ID_VIEW_SHOWINTERIORLINKS, MF_CHECKED);
				}
				break;
			case ID_ADD_INTERIORLINK:
				InteriorMapEditorAddNewRegion("InteriorLink", "");
				break;
			case ID_ADD_BUILDINGACCESSREGIONS_SALESREGION:
				InteriorMapEditorAddNewRegion("SalesPoint", "");
				break;
			case ID_ADD_BUILDINGACCESSREGIONS_STOCKSREGION:
				InteriorMapEditorAddNewRegion("StocksPoint", "");
				break;
			case ID_ADD_BUILDINGACCESSREGIONS_BUILDINGACCESSREGION:
				InteriorMapEditorAddNewRegion("BuildingAccess", "");
				break;
			case ID_ADD_EXITPOINT:
				InteriorMapEditorAddNewRegion("ExitPoint", "");
				break;
			case ID_ADD_SPAWNPOINT:
				InteriorMapEditorAddNewRegion("SpawnPoint", "");
				break;
			case ID_ADD_SCRIPTEVENT:
				InteriorMapEditorAddNewRegion("ScriptEvent", "");
				break;
			case ID_ADD_ROOM:
				InteriorMapEditorAddNewRoom();
				break;
			case ID_ADD_FURNITURE:
				InteriorMapEditorAddNewFurniture();
				break;
			case IDM_EDIT_UNDO:
				MapEdUndo();
				break;
			case ID_RENDERING_LIGHTING_100AMBIENT:
				CheckMenuItem(GetMenu(hDlg), ID_RENDERING_LIGHTING_50AMBIENT50DIRECTIONAL, MF_UNCHECKED);
				CheckMenuItem(GetMenu(hDlg), ID_RENDERING_LIGHTING_100DIRECTIONAL, MF_UNCHECKED);
				CheckMenuItem(GetMenu(hDlg), ID_RENDERING_LIGHTING_100AMBIENT, MF_CHECKED);
				InteriorMapEditorUpdateLightingMode(1);				
				break;
			case ID_RENDERING_LIGHTING_50AMBIENT50DIRECTIONAL:
				CheckMenuItem(GetMenu(hDlg), ID_RENDERING_LIGHTING_50AMBIENT50DIRECTIONAL, MF_CHECKED);
				CheckMenuItem(GetMenu(hDlg), ID_RENDERING_LIGHTING_100DIRECTIONAL, MF_UNCHECKED);
				CheckMenuItem(GetMenu(hDlg), ID_RENDERING_LIGHTING_100AMBIENT, MF_UNCHECKED);
				InteriorMapEditorUpdateLightingMode(0);				
				break;
			case ID_RENDERING_LIGHTING_100DIRECTIONAL:
				CheckMenuItem(GetMenu(hDlg), ID_RENDERING_LIGHTING_50AMBIENT50DIRECTIONAL, MF_UNCHECKED);
				CheckMenuItem(GetMenu(hDlg), ID_RENDERING_LIGHTING_100DIRECTIONAL, MF_CHECKED);
				CheckMenuItem(GetMenu(hDlg), ID_RENDERING_LIGHTING_100AMBIENT, MF_UNCHECKED);
				InteriorMapEditorUpdateLightingMode(2);				
				break;
			case IDC_CHECK1:
				nVal = 	SendDlgItemMessage( hDlg, IDC_CHECK1, BM_GETCHECK, 0, 0 );
				if ( nVal )
				{
					mbXAxisActive = TRUE;
				}
				else
				{
					mbXAxisActive = FALSE;
				}
				break;
			case IDC_CHECK2:
				nVal = 	SendDlgItemMessage( hDlg, IDC_CHECK2, BM_GETCHECK, 0, 0 );
				if ( nVal )
				{
					mbYAxisActive = TRUE;
				}
				else
				{
					mbYAxisActive = FALSE;
				}
				break;
			case IDC_CHECK3:
				nVal = 	SendDlgItemMessage( hDlg, IDC_CHECK3, BM_GETCHECK, 0, 0 );
				if ( nVal )
				{
					mbZAxisActive = TRUE;
				}
				else
				{
					mbZAxisActive = FALSE;
				}
				break;
			case IDM_FILE_EXIT:
				PostQuitMessage(0);
				break;
			case IDM_FILE_NEW:
				SceneMapReset();
				SceneMapInitNew();
				InteriorMapEditorInitScene();
				InteriorMapEditorResetCamera();
				MapEditorRefreshTreeview();
				break;
			case ID_FILE_SAVEARCHIVE:
				acFilename[0] = 0;
				if ( SysGetSaveFilenameDialog( "Universal Model Archive(*.uma)\0*.uma\0All Files(*.*)\0*.*\0", "Save Map Archive", mszDefaultFilePath, 0, acFilename ) > 0 )
				{
					SceneMapSaveArchive( acFilename );
				}

				break;
			case ID_EDIT_PROPERTIES:
				DialogBox(ghInstance, (LPCTSTR)IDD_EDIT_PROPERTIES, NULL, (DLGPROC)EditPropertiesDlgProc );
				break;
			case IDM_FILE_SAVEAS:
				acFilename[0] = 0;
				if ( SysGetSaveFilenameDialog( "Universal Interior Map File(*.uim)\0*.uim\0All Files(*.*)\0*.*\0", "Save Interior Map File", mszDefaultFilePath, 0, acFilename ) > 0 )
				{
					InteriorMapEditorStoreDefaultFilepath(acFilename);
					SceneMapSave( acFilename );
				}
				break;
			case IDM_FILE_OPEN:
				acFilename[0] = 0;
				if (SysGetOpenFilenameDialog( "Universal Interior Map File(*.uim)\0*.uim\0Universal Model Archive(*.uma)\0*.uma\0All Files(*.*)\0*.*\0", "Select interior map file", mszDefaultFilePath, 0, acFilename ))
				{
					InteriorMapEditorStoreDefaultFilepath(acFilename);
					SceneMapReset();
					SceneMapLoad( acFilename, NULL, "" );
					MapEditorRefreshTreeview();
				}
				break;
			case IDC_CENTRE_BUT:
				MapEditorCentreOnSelected();
				break;
			case IDC_MOVE_BUT:
				MouseCamSetControlMode( CONTROL_MOVE );
				UpdateControlIcons();
				break;
			case IDC_ROT_BUT:
				MouseCamSetControlMode( CONTROL_ROT );
				UpdateControlIcons();
				break;
			case IDC_ZOOM_BUT:
				MouseCamSetControlMode( CONTROL_ZOOM );
				UpdateControlIcons();
				break;
			}
			break;
		}
		break;
	case WM_LBUTTONUP:

		break;
	case WM_SIZE:
		mboMinimised = FALSE;
		switch ( wParam )
		{
		case SIZE_MAXIMIZED:
			MapEditorResizeWindow( LOWORD(lParam), HIWORD(lParam) );
			break;
		case SIZE_RESTORED:
//			MapEditorResizeWindow( LOWORD(lParam), HIWORD(lParam) );
			break;
		case SIZE_MINIMIZED:
			mboMinimised = TRUE;
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


//--------------------------------------------------------------------
// WinMain
//     Application entry point
//--------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

INITCOMMONCONTROLSEX	xCommStruct;
 	// TODO: Place code here.

	xCommStruct.dwSize = sizeof( xCommStruct );
	xCommStruct.dwICC = ICC_LISTVIEW_CLASSES|ICC_TREEVIEW_CLASSES;
	InitCommonControlsEx( &xCommStruct );
	InteriorMapEditorInitDefaultFilepath();

	ghInstance = hInstance;

	mhbmpMoveSel = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP4)); 
	mhbmpRotSel = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP5)); 	
	mhbmpZoomSel = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP6)); 
	mhbmpMove = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP3)); 
	mhbmpRot = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP2)); 	
	mhbmpZoom = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP1)); 

	DialogBox(hInstance, (LPCTSTR)IDD_DIALOG1, NULL, (DLGPROC)MapEditorMainDlgProc );

	// Clear everything up
	EngineFree(TRUE);
	InterfaceFree();
	InterfaceFreeAllD3D();

//    UnregisterClass( mszProjectClassName, ghInstance );

	return 0;
}
