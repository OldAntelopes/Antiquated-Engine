#include <stdio.h>
#include <windows.h>
#include <CommCtrl.h>
#include "../resource.h"

#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../../Pub/LibCode/Engine/ModelRendering.h"		// TEMP - Shouldnt be doing this :)

#include "../ModelConverter.h"


int		mnTurretAttachEditNum = 0;
int		mnAttachDialogSelectedVertexNum = NOTFOUND;
HWND	mhwndTurretAttachDlg;
int		mnShowBall = 0;


int		AttachDlgBallAttachPoint( void )
{
	return( mnShowBall );
}

void TurretAttachDlgInitLists( void )
{
char	acString[256];
MODEL_RENDER_DATA*	pxModelData;

	if ( ModelConvGetCurrentModel() != NOTFOUND )
	{
		pxModelData = maxModelRenderData + ModelConvGetCurrentModel();

		SendDlgItemMessage( mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETBUDDY, (WPARAM)GetDlgItem(mhwndTurretAttachDlg,IDC_EDIT1), 0L);
		SendDlgItemMessage (mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETRANGE, 0L,MAKELONG (pxModelData->xStats.nNumVertices - 1, 0));
		switch( mnTurretAttachEditNum )
		{	
		case 0:
		default:
			if ( mnAttachDialogSelectedVertexNum != NOTFOUND )
			{
				pxModelData->xHorizTurretData.nAttachVertex = mnAttachDialogSelectedVertexNum;
			}
			sprintf( acString, "%d", pxModelData->xHorizTurretData.nAttachVertex );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT1, acString );
			SendDlgItemMessage (mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETPOS, 0L,MAKELONG (pxModelData->xHorizTurretData.nAttachVertex, 0));

			sprintf( acString, "%.4f", pxModelData->xHorizTurretData.xAttachOffset.x );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT2, acString );
			sprintf( acString, "%.4f", pxModelData->xHorizTurretData.xAttachOffset.y );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT3, acString );
			sprintf( acString, "%.4f", pxModelData->xHorizTurretData.xAttachOffset.z );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT4, acString );
			break;
		case 1:
			if ( mnAttachDialogSelectedVertexNum != NOTFOUND )
			{
				pxModelData->xVertTurretData.nAttachVertex = mnAttachDialogSelectedVertexNum;
			}
			sprintf( acString, "%d", pxModelData->xVertTurretData.nAttachVertex );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT1, acString );
			SendDlgItemMessage (mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETPOS, 0L,MAKELONG (pxModelData->xVertTurretData.nAttachVertex, 0));

			sprintf( acString, "%.4f", pxModelData->xVertTurretData.xAttachOffset.x );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT2, acString );
			sprintf( acString, "%.4f", pxModelData->xVertTurretData.xAttachOffset.y );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT3, acString );
			sprintf( acString, "%.4f", pxModelData->xVertTurretData.xAttachOffset.z );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT4, acString );
			break;
		case 2:
			if ( mnAttachDialogSelectedVertexNum != NOTFOUND )
			{
				pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex = mnAttachDialogSelectedVertexNum;
			}
			sprintf( acString, "%d", pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT1, acString );
			SendDlgItemMessage (mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETPOS, 0L,MAKELONG (pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex, 0));

			sprintf( acString, "%.4f", pxModelData->xAttachData.xGenericWeaponFireAttach.xAttachOffset.x );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT2, acString );
			sprintf( acString, "%.4f", pxModelData->xAttachData.xGenericWeaponFireAttach.xAttachOffset.y );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT3, acString );
			sprintf( acString, "%.4f", pxModelData->xAttachData.xGenericWeaponFireAttach.xAttachOffset.z );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT4, acString );
			break;
		case 3:
			if ( mnAttachDialogSelectedVertexNum != NOTFOUND )
			{
				pxModelData->xWheel1AttachData.nAttachVertex = mnAttachDialogSelectedVertexNum;
			}
			sprintf( acString, "%d", pxModelData->xWheel1AttachData.nAttachVertex );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT1, acString );
			SendDlgItemMessage (mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETPOS, 0L,MAKELONG (pxModelData->xWheel1AttachData.nAttachVertex, 0));

			sprintf( acString, "%.4f", pxModelData->xWheel1AttachData.xAttachOffset.x );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT2, acString );
			sprintf( acString, "%.4f", pxModelData->xWheel1AttachData.xAttachOffset.y );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT3, acString );
			sprintf( acString, "%.4f", pxModelData->xWheel1AttachData.xAttachOffset.z );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT4, acString );
			break;
		case 4:
			if ( mnAttachDialogSelectedVertexNum != NOTFOUND )
			{
				pxModelData->xWheel2AttachData.nAttachVertex = mnAttachDialogSelectedVertexNum;
			}
			sprintf( acString, "%d", pxModelData->xWheel2AttachData.nAttachVertex );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT1, acString );
			SendDlgItemMessage (mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETPOS, 0L,MAKELONG (pxModelData->xWheel2AttachData.nAttachVertex, 0));

			sprintf( acString, "%.4f", pxModelData->xWheel2AttachData.xAttachOffset.x );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT2, acString );
			sprintf( acString, "%.4f", pxModelData->xWheel2AttachData.xAttachOffset.y );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT3, acString );
			sprintf( acString, "%.4f", pxModelData->xWheel2AttachData.xAttachOffset.z );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT4, acString );
			break;
		case 5:
			if ( mnAttachDialogSelectedVertexNum != NOTFOUND )
			{
				pxModelData->xWheel3AttachData.nAttachVertex = mnAttachDialogSelectedVertexNum;
			}
			sprintf( acString, "%d", pxModelData->xWheel3AttachData.nAttachVertex );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT1, acString );
			SendDlgItemMessage (mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETPOS, 0L,MAKELONG (pxModelData->xWheel3AttachData.nAttachVertex, 0));

			sprintf( acString, "%.4f", pxModelData->xWheel3AttachData.xAttachOffset.x );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT2, acString );
			sprintf( acString, "%.4f", pxModelData->xWheel3AttachData.xAttachOffset.y );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT3, acString );
			sprintf( acString, "%.4f", pxModelData->xWheel3AttachData.xAttachOffset.z );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT4, acString );
			break;
		case 6:
			if ( mnAttachDialogSelectedVertexNum != NOTFOUND )
			{
				pxModelData->xWheel4AttachData.nAttachVertex = mnAttachDialogSelectedVertexNum;
			}
			sprintf( acString, "%d", pxModelData->xWheel4AttachData.nAttachVertex );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT1, acString );
			SendDlgItemMessage (mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETPOS, 0L,MAKELONG (pxModelData->xWheel4AttachData.nAttachVertex, 0));

			sprintf( acString, "%.4f", pxModelData->xWheel4AttachData.xAttachOffset.x );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT2, acString );
			sprintf( acString, "%.4f", pxModelData->xWheel4AttachData.xAttachOffset.y );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT3, acString );
			sprintf( acString, "%.4f", pxModelData->xWheel4AttachData.xAttachOffset.z );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT4, acString );
			break;
		case 7:
			if ( mnAttachDialogSelectedVertexNum != NOTFOUND )
			{
				pxModelData->xEffectAttachData.nAttachVertex = mnAttachDialogSelectedVertexNum;
			}
			sprintf( acString, "%d", pxModelData->xEffectAttachData.nAttachVertex );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT1, acString );
			SendDlgItemMessage (mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETPOS, 0L,MAKELONG (pxModelData->xEffectAttachData.nAttachVertex, 0));

			sprintf( acString, "%.4f", pxModelData->xEffectAttachData.xAttachOffset.x );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT2, acString );
			sprintf( acString, "%.4f", pxModelData->xEffectAttachData.xAttachOffset.y );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT3, acString );
			sprintf( acString, "%.4f", pxModelData->xEffectAttachData.xAttachOffset.z );
			SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT4, acString );
			break;
		}
	}
}

void EffectAttachDlgInitLists( void )
{
char	acString[256];
MODEL_RENDER_DATA*	pxModelData;
//int		nSelectedVertexNum;

	pxModelData = maxModelRenderData + ModelConvGetCurrentModel();

	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_RESETCONTENT, 0, 0 );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"<None>" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Black Smoke" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"White Smoke" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Campfire" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Flashing Purple Neon" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Soft light" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Grey smoke" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Soft light - Nighttime only" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Large grey smoke" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Fountain small" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Fountain large" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Particle Fountain small" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Particle Fountain large" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Oil vapour flame" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Unknown effect (14)" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Unknown effect (15)" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Unknown effect (16)" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)"Unknown effect (17)" );
	SendDlgItemMessage( mhwndTurretAttachDlg, IDC_EFFECT_TYPE_COMBO, CB_SETCURSEL, pxModelData->xEffectAttachData.nEffectType, 0 );
//	sprintf( acString, "%d", pxModelData->xEffectAttachData.nEffectType );
//	SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT5, acString );
	sprintf( acString, "%d", pxModelData->xEffectAttachData.ulEffectParam1 );
	SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT6, acString );
	sprintf( acString, "%d", pxModelData->xEffectAttachData.ulEffectParam2 );
	SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT7, acString );

	if ( mnAttachDialogSelectedVertexNum != NOTFOUND )
	{
		pxModelData->xEffectAttachData.nAttachVertex = mnAttachDialogSelectedVertexNum;
	}
	sprintf( acString, "%d", pxModelData->xEffectAttachData.nAttachVertex );
	SetDlgItemText( mhwndTurretAttachDlg, IDC_EDIT1, acString );
	SendDlgItemMessage (mhwndTurretAttachDlg, IDC_SPIN1, UDM_SETPOS, 0L,MAKELONG (pxModelData->xEffectAttachData.nAttachVertex, 0));

	
}

int		GetValueForNotatedString( const char* szStringValue )
{
char	acString[256];
const char*		pcRunner = szStringValue;
char*	pcOutRunner = acString;
int		nVal;

	while( *pcRunner != 0 )
	{
		if ( *pcRunner != ',' )
		{
			*pcOutRunner = *pcRunner;
			pcOutRunner++;
		}
		pcRunner++;
	}
	nVal = strtol( acString, NULL, 10 );
	return( nVal );
}

LRESULT CALLBACK EffectAttachPointDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
NMUPDOWN*	lpnmud;
MODEL_RENDER_DATA*	pxModelData;

	switch (message)
	{
	case WM_INITDIALOG:
		mhwndTurretAttachDlg = hDlg;
		TurretAttachDlgInitLists();
		EffectAttachDlgInitLists();
		mnShowBall = 2;
		return TRUE;
	case WM_NOTIFY:
		lpnmud = (LPNMUPDOWN)(lParam);
		if ( ModelConvGetCurrentModel() != NOTFOUND )
		{
//		char	acString[256];

			pxModelData = maxModelRenderData + ModelConvGetCurrentModel();
			pxModelData->xEffectAttachData.nAttachVertex = lpnmud->iPos + lpnmud->iDelta;
			break;
		}
		break;
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
				int		nVal;

					pxModelData = maxModelRenderData + ModelConvGetCurrentModel();			
					if ( pxModelData->xEffectAttachData.nAttachVertex != 0 )
					{
						pxModelData->bHasEffect = 1;
					}

					nVal = SendDlgItemMessage( hDlg, IDC_EFFECT_TYPE_COMBO, CB_GETCURSEL, 0, 0 );
					pxModelData->xEffectAttachData.nEffectType = nVal;
					GetDlgItemText( hDlg, IDC_EDIT6, acString, 256 );
					pxModelData->xEffectAttachData.ulEffectParam1 = strtol( acString, NULL, 10 );
					GetDlgItemText( hDlg, IDC_EDIT7, acString, 256 );
					pxModelData->xEffectAttachData.ulEffectParam2 = strtol( acString, NULL, 10 );
					EndDialog(hDlg, 0);
				}
				break;
			case IDCANCEL:
				EndDialog(hDlg, 0);
				break;
			}
			break;
		case EN_CHANGE:
			switch( LOWORD(wParam) )
			{
			case IDC_EDIT1:
				{
				char	acString[256];
				int		nVal;

					GetDlgItemText( hDlg, LOWORD(wParam), acString, 256);
					nVal = GetValueForNotatedString( acString );
					if ( ModelConvGetCurrentModel() != NOTFOUND )
					{
						pxModelData = maxModelRenderData + ModelConvGetCurrentModel();
						if ( nVal >= pxModelData->xStats.nNumVertices ) nVal = pxModelData->xStats.nNumVertices - 1;
						pxModelData->xEffectAttachData.nAttachVertex = nVal;
					}
				}
				break;
			}
			break;
		}
		break;
	case WM_CLOSE:
		mnShowBall = 0;
		EndDialog(hDlg, LOWORD(wParam));
		return( 0 );
	case WM_DESTROY:
		mnShowBall = 0;
		return(0);
	default:
		break;
	}
	return( FALSE );
}


void	SetCurrentAttachPointVertexNum( int nValue )
{
MODEL_RENDER_DATA*	pxModelData;

	if ( ModelConvGetCurrentModel() != NOTFOUND )
	{
//		char	acString[256];
			
		pxModelData = maxModelRenderData + ModelConvGetCurrentModel();
		switch( mnTurretAttachEditNum )
		{	
		case 0:
		default:
			pxModelData->xHorizTurretData.nAttachVertex = nValue;
//				sprintf( acString, "horiz turret set to vertex %d", pxModelData->xHorizTurretData.nAttachVertex );
//				SendDlgItemMessage( mhwndMainDialog, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)( acString ) );
			break;
		case 1:
			pxModelData->xVertTurretData.nAttachVertex = nValue;
			break;
		case 2:
			pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex = nValue;
			break;
		case 3:
			pxModelData->xWheel1AttachData.nAttachVertex = nValue;
			break;
		case 4:
			pxModelData->xWheel2AttachData.nAttachVertex = nValue;
			break;
		case 5:
			pxModelData->xWheel3AttachData.nAttachVertex = nValue;
			break;
		case 6:
			pxModelData->xWheel4AttachData.nAttachVertex = nValue;
			break;
		case 7:
			pxModelData->xEffectAttachData.nAttachVertex = nValue;
			if ( pxModelData->xEffectAttachData.nAttachVertex != 0 )
			{
				pxModelData->bHasEffect = 1;
			}
			break;
		}
	}

}

/***************************************************************************
 * Function    : TurretAttachPointDlg
 * Params      :
 * Returns     :
 * Description :
 ***************************************************************************/
LRESULT CALLBACK TurretAttachPointDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
NMUPDOWN*	lpnmud;

	switch (message)
	{
	case WM_INITDIALOG:
		mhwndTurretAttachDlg = hDlg;
		if ( mnTurretAttachEditNum == 2 )	// generic weapon fire attach point
		{
			mnShowBall = 1;
		}
		else if ( mnTurretAttachEditNum == 1 )		// vert turret attach point
		{
			mnShowBall = 3;
		}
		else if ( mnTurretAttachEditNum == 0 )		// horiz turret attach point
		{
			mnShowBall = 4;
		}
		TurretAttachDlgInitLists();
		return TRUE;
	case WM_NOTIFY:
		lpnmud = (LPNMUPDOWN)(lParam);

		SetCurrentAttachPointVertexNum( lpnmud->iPos + lpnmud->iDelta );
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDOK:
				EndDialog(hDlg, 0);
				break;
			case IDCANCEL:
				EndDialog(hDlg, 0);
				break;
			}
		case EN_CHANGE:
			switch( LOWORD(wParam) )
			{
			case IDC_EDIT1:
				{
				char	acString[256];
				int		nVal;

					GetDlgItemText( hDlg, LOWORD(wParam), acString, 256);
					nVal = GetValueForNotatedString( acString );
					if ( ModelConvGetCurrentModel() != NOTFOUND )
					{
					MODEL_RENDER_DATA*		pxModelData = maxModelRenderData + ModelConvGetCurrentModel();
						if ( nVal >= pxModelData->xStats.nNumVertices )
						{
							nVal = pxModelData->xStats.nNumVertices - 1;
						}
					}
					SetCurrentAttachPointVertexNum( nVal );
				}
				break;
			}
			break;
		}
		break;
	case WM_CLOSE:
		mnShowBall = 0;
		EndDialog(hDlg, LOWORD(wParam));
		return( 0 );
	case WM_DESTROY:
		mnShowBall = 0;
		return(0);
	default:
		break;
	}
	return( FALSE );
}



void TurretAttachDlgInit( int nTurretNum, int nVertexNum )
{
	mnTurretAttachEditNum = nTurretNum;	
	mnAttachDialogSelectedVertexNum = nVertexNum;
	if ( nTurretNum == 7 )	// if effect attach
	{
		DialogBox(ghInstance, (LPCTSTR)IDD_EFFECT_ATTACH, NULL, (DLGPROC)EffectAttachPointDlg );		
	}
	else
	{
		DialogBox(ghInstance, (LPCTSTR)IDD_TURRET_ATTACH_POINT, NULL, (DLGPROC)TurretAttachPointDlg );		
	}

}
