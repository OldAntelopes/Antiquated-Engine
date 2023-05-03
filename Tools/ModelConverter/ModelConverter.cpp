
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <CommCtrl.h>
#include <shlobj.h>
#include "resource.h"

#include <d3dx9.h>
#include <dxfile.h>


#include <StandardDef.h>
#include <System.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#ifdef SECURITY_FEATURES
#include <Security.h>
#endif

#include "../../../GameCommon/RenderUtil/Sprites3D.h"

#include "../LibCode/Engine/ModelFiles.h"
#include "../LibCode/Engine/ModelRendering.h"	
#include "../LibCode/Engine/ModelMaterialData.h"		
#include "../LibCode/Engine/DirectX/ShadowVolumeDX.h"	

#include "Util/Line Renderer.h"
#include "Util/Point Renderer.h"
#include "Scene/ModelEditorSceneObject.h"
#include "Dialogs/CreateSceneFromModel.h"
#include "Dialogs/MaterialsDialog.h"
#include "Dialogs/AttachDialog.h"
#include "AddPrimitives/AddPrimitives.h"
#include "AddPrimitives/CurvedWall.h"
#include "Tools/RecalcNormals.h"
#include "Tools/MeshManipulate.h"
#include "Tools/ScaleModel.h"
#include "Tools/MoveModel.h"
#include "Tools/UVGenerate.h"
#include "Tools/PlaneCutter.h"
#include "Tools/VertexColours.h"
#include "Tools/VertexManipulate.h"
#include "RenderEffects\RenderEffects.h"

#include "CombineMaterials.h"
#include "ModelConverter.h"
#include "Animation Builder.h"
#include "MaterialBrowser.h"
#include "SubModelBrowser.h"
#include "UVUnwrap.h"

#define	MODEL_CONV_NO_LOCK_PROTECTED 

enum
{
	CONTROL_CAMERA,
	CONTROL_PICKER,
	CONTROL_MOVE,
	CONTROL_ROT,
	CONTROL_ZOOM,
	CONTROL_VERTEXPICKER,
	CONTROL_POLYSLICE,
	CONTROL_VERTEXMOVE,
	CONTROL_VERTEXROTATE,
	CONTROL_VERTEXSCALE,
};

HINSTANCE	ghInstance;

HWND		mhwndMainDialog;
HWND		mhwndExportSettingsDialog;

HWND	mhwndBatchConvertDialog;

CSceneObject	m_MainSceneObject;

#define	MODELCONV_VERSIONSTRING		"2.34"

BOOL	mbDisableWASD = FALSE;
int		mnCurrentWheelMode = 0;
int		mnTemporaryDisplayModelHandle = NOTFOUND;
int		mnHorizTurretModelHandle = NOTFOUND;
int		mnVertTurretModelHandle = NOTFOUND;
int		mnWheel1ModelHandle = NOTFOUND;
int		mnWheel2ModelHandle = NOTFOUND;
int		mnLowLODModelHandle = NOTFOUND;
int		mnMedLODModelHandle = NOTFOUND;
int		mnCollisionModelHandle = NOTFOUND;
int		mnModelRenderingMode = 0;
int		mnPolysRendered = 0;
int		mnLightingMode = 0;
int		mnLastDisplayPolyRendered = 0;
int		mnLastDisplaySelectionWindow = 0;

LPDIRECT3DDEVICE9       g_pd3dDevice = NULL;
VECT	mxMousePos;
VECT	mxMouseDownPos;
BOOL	mboLeftMouseDown = FALSE;
BOOL	mboRightMouseDown = FALSE;
BOOL	mboMidMouseDown = FALSE;
BOOL	mboMinimised = FALSE;
HBITMAP 	mhbmpMoveSel;
HBITMAP 	mhbmpRotSel;
HBITMAP 	mhbmpZoomSel;
HBITMAP 	mhbmpMove;
HBITMAP 	mhbmpRot;
HBITMAP 	mhbmpZoom;

BOOL	mboDrawGridLines = TRUE;
BOOL	mboShowNormals = FALSE;
char	mszCommandLineFile[256];

int		mnLODUsedDisplay = -1;
int		mnBackgroundColour = 0;
uint32	mulLastExportFlags = ATM_EXPORT_FLAGS_COMPRESSION | ATM_EXPORT_FLAGS_VERY_COMPRESSED_UVS | ATM_EXPORT_FLAGS_COMPRESSED_VERTICES | ATM_EXPORT_FLAGS_MATERIALS;

char	macBatchConvFolder[256];
char	macLoadedTextureFilename[256];

char	maszCurrentModelFilename[256];

int		mnPickerControlMode = CONTROL_CAMERA;
int		mnViewPage = 0;
BOOL	mboViewIsAnimation = TRUE;
BOOL	mboIsResizing = FALSE;
LRESULT CALLBACK ModelConverterMainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

float	mfCamDist = 1.0f;
float	mfOriginalCamDist = 1.0f;

VECT		mxBaseCamFocus  = { 0.0f, 0.0f, 0.05f };
VECT		mxCamFocus  = { 0.0f, 0.0f, 0.05f };
VECT		mxCamPos  = { 0.0f, 1.0f, 0.0f };
VECT		mxCamDir = { 0.0f, -1.0f, 0.0f };

VECT	mxOriginalCamView;
float	mfOriginalViewAngleVert;
float	mfViewAngleVert = 0.0f;
float	mfOriginalViewAngleHoriz;
float	mfViewAngleHoriz = 0.0f;
float	mfWheelTurnRot = 0.0f;

BOOL	mbModelConvShowSelectionRect = FALSE;
BOOL	mbModelConvMoveLockX = FALSE;
BOOL	mbModelConvMoveLockY = FALSE;
BOOL	mbModelConvMoveLockZ = FALSE;

int		mnBallModelHandle = NOTFOUND;
HWND	mhwndGraphicWindow = NULL;

int		mnNumLinesTextAdded = 0;

uint32	gulLastTick = 0;

MVECT				mxCamVect;
MVECT				mxBaseCamPos;

char	macModelConvRootFolder[256];
char	macModelConvLastSaveFolder[256];
char	macModelConvLastLoadFolder[256];

BOOL	mbThisWindowIsFocused = FALSE;


//------------------------------------------------------------

#define		SIZE_OF_ONE_TILE		(115.0f/256.0f)
float		mfFloorSize = SIZE_OF_ONE_TILE * 5.0f;
float		mfFloorSteps = SIZE_OF_ONE_TILE;

float		mfTractorUnitScale = 0.159659f;
// -- For TheUniversal stuff.. whacky ft scale
//float		mfScaleUnit = (0.159659f/12.0f);		// i.e. 1ft = 0.013310491666,  1.0f = 75ft
//int			mnScaleUnitMode = 0;

// -- Normal (For A&E)
float		mfScaleUnit = 1.0f;
int			mnScaleUnitMode = 1;

void		ModelConverterSetupCamera( void );


//----------------------------------
class ModelConvViewInterface : public CViewInterface
{
public:
	virtual void	GetScreenCoordForWorldCoord( VECT* pxWorldCoord, int* pnX, int* pnY )
	{
	D3DXMATRIX matProj;
	D3DXMATRIX matView;
	D3DXMATRIX matWorld;
	D3DXVECTOR3	xVect;
	int		nMidX;
	int		nMidY;

	int		mnViewX = 0;
	int		mnViewY = 0;
	int		mnViewWidth = InterfaceGetWidth();
	int		mnViewHeight = InterfaceGetHeight();

		xVect.x = 0.0f;
		xVect.y = 0.0f;
		xVect.z = 0.0f;

		ModelConverterSetupCamera();
		g_pd3dDevice->GetTransform( D3DTS_PROJECTION, &matProj );
		g_pd3dDevice->GetTransform( D3DTS_VIEW, &matView );
		g_pd3dDevice->GetTransform( D3DTS_WORLD, &matWorld );
		
		D3DXVec3Project( &xVect, (D3DXVECTOR3*)( pxWorldCoord ), NULL, &matProj, &matView, &matWorld );

		/** Doing my own viewport calculations.. dont know why.. **/
		nMidX = mnViewWidth / 2;
		nMidY = mnViewHeight / 2;

		*(pnX) = (int)( nMidX * xVect.x ) + (nMidX+mnViewX);
		*(pnY) = (int)( nMidY * -xVect.y ) + (nMidY+mnViewY);
	}

	virtual void	GetRayForScreenCoord( int nScreenX, int nScreenY, VECT* pxRayStart, VECT* pxRayDir )
	{
	D3DXMATRIX matProj;
	D3DXMATRIX matView;
	D3DXMATRIX matWorld;
	D3DXMATRIX m;
	D3DXVECTOR3	xVect;
	//int		nMidX;
	//int		nMidY;
	POINT ptCursor;

	int		mnViewX = 0;
	int		mnViewY = 0;
	int		mnViewWidth = InterfaceGetWidth();//InterfaceGetWindowWidth();
	int		mnViewHeight = InterfaceGetHeight();//InterfaceGetWindowHeight();

		xVect.x = 0.0f;
		xVect.y = 0.0f;
		xVect.z = 0.0f;
	
		ModelConverterSetupCamera();
		g_pd3dDevice->GetTransform( D3DTS_PROJECTION, &matProj );
		g_pd3dDevice->GetTransform( D3DTS_VIEW, &matView );

		ptCursor.x = nScreenX - mnViewX;
		ptCursor.y = nScreenY - mnViewY;

		// Compute the vector of the pick ray in screen space
		D3DXVECTOR3 v;
		v.x =  ( ( ( 2.0f * ptCursor.x ) / mnViewWidth ) - 1 ) / matProj._11;
		v.y = -( ( ( 2.0f * ptCursor.y ) / mnViewHeight ) - 1 ) / matProj._22;
		v.z =  1.0f;

		// Get the inverse view matrix
		D3DXMatrixInverse( &m, NULL, &matView );

		// Transform the screen space pick ray into 3D space
		pxRayDir->x  = v.x*m._11 + v.y*m._21 + v.z*m._31;
		pxRayDir->y  = v.x*m._12 + v.y*m._22 + v.z*m._32;
		pxRayDir->z  = v.x*m._13 + v.y*m._23 + v.z*m._33;
		pxRayStart->x = m._41;
		pxRayStart->y = m._42;
		pxRayStart->z = m._43;
	}
};

ModelConvViewInterface		m_sModelConvViewInterface;

void	ModelConvTextBoxSet( const char* format, ... )
{
char		acMessage[2048];
va_list		marker;
uint32*		pArgs;
BOOL		bEndsWithNewLine = FALSE;

	pArgs = (uint32*)( &format ) + 1;

	va_start( marker, format );     
	vsprintf( acMessage, format, marker );

	SetDlgItemText( mhwndMainDialog, IDC_LEFT_TEXTBOX, acMessage );
}

void	ModelConvTextAdd( const char* format, ... )
{
char		acMessage[2048];
va_list		marker;
uint32*		pArgs;
BOOL		bEndsWithNewLine = FALSE;

	pArgs = (uint32*)( &format ) + 1;

	va_start( marker, format );     
	vsprintf( acMessage, format, marker );


	SendDlgItemMessage( mhwndMainDialog, IDC_OUTPUT_LIST, LB_ADDSTRING, 0, (LPARAM)( acMessage ) );
	SendDlgItemMessage( mhwndMainDialog, IDC_OUTPUT_LIST, LB_SETTOPINDEX, (WPARAM)(mnNumLinesTextAdded), 0 );
	mnNumLinesTextAdded++;
}

void	ModelConvSaveUsedDirectories( void )
{
FILE*	pFile;
char	acString[256];
	sprintf( acString, "%s/modelconvdirs.opt", macModelConvRootFolder );
	pFile = fopen( acString, "wb" );
	if ( pFile != NULL )
	{
		fwrite( macModelConvLastSaveFolder, 256, 1, pFile );
		fwrite( macModelConvLastLoadFolder, 256, 1, pFile );
		fclose( pFile );
	}
}


MVECT*	GetCameraPos( void )
{
	return( (MVECT*)&mxCamPos );
}


void		GetPathFromFilename( const char* szFullFilename, char* szPathOut )
{
char*		pcRunner;

	strcpy( szPathOut, szFullFilename );
	pcRunner = szPathOut + strlen( szPathOut );

	while ( ( *pcRunner != '\\' ) &&
		    ( *pcRunner != '/' ) &&
			( pcRunner > szPathOut ) )
	{
		pcRunner--;
	}

	if ( ( *pcRunner == '\\' ) ||
		 ( *pcRunner == '/' ) )
	{
		*pcRunner = 0;
	}

}



/***************************************************************************
 * Function    : SetCameraPos
 ***************************************************************************/
MVECT*	GetCameraBasePos( void )
{
	return( (MVECT*)&mxBaseCamPos );
}


void		ModelConvPrintToConsole( const char* szText )
{
	ModelConvTextAdd( szText );
}

void		DebugPrint( int mode, const char* format, ... )
{
char            acString[512]; 
va_list         marker; 

    va_start( marker, format );    
	vsprintf( acString, format, marker ); 
	ModelConvPrintToConsole( acString );
}

/***************************************************************************
 * Function    : SetCameraPos
 ***************************************************************************/
void	SetCameraPos( float fX, float fY, float fZ )
{
	mxBaseCamPos.x = fX;
	mxBaseCamPos.y = fY;
	mxBaseCamPos.z = fZ;

#ifndef TOOL
	CameraAddShakeToBase();
#endif
}

/***************************************************************************
 * Function    : CameraSetPosIgnoreShake
 ***************************************************************************/
void	CameraSetPosIgnoreShake( float fX, float fY, float fZ )
{
	mxBaseCamPos.x = fX;
	mxBaseCamPos.y = fY;
	mxBaseCamPos.z = fZ;
	mxCamPos.x = fX;
	mxCamPos.y = fY;
	mxCamPos.z = fZ;
}

void ModelConvSetWheelModeOption( int mode )
{
	if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
	{					
		ModelRenderSetWheelDisplayMode( m_MainSceneObject.GetModelHandle(), mode, 0.0f );
		mnCurrentWheelMode = mode;
	}

	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_NORMAL, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_PLANETYPE1, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_PLANETYPE2, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_HELICOPTER, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_HELICOPTER4, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_BIKE, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_WHEELS_YFORWARD, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_BIKE_YFORWARD, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_PLANETYPE1_YFORWARD, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_PLANETYPE2_YFORWARD, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_WHEELS_YFORWARD_NOSTEER, MF_UNCHECKED);
	
	
	switch( mode )
	{
	case 0:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_NORMAL, MF_CHECKED);
		break;
	case 1:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_PLANETYPE1, MF_CHECKED);
		break;
	case 2:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_PLANETYPE2, MF_CHECKED);
		break;
	case 3:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_HELICOPTER, MF_CHECKED);
		break;
	case 4:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_HELICOPTER4, MF_CHECKED);
		break;
	case 5:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_BIKE, MF_CHECKED);
		break;
	case 6:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_WHEELS_YFORWARD, MF_CHECKED);
		break;		
	case 7:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_BIKE_YFORWARD, MF_CHECKED);
		break;		
	case 8:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_PLANETYPE1_YFORWARD, MF_CHECKED);
		break;		
	case 9:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_PLANETYPE2_YFORWARD, MF_CHECKED);
		break;		
	case 10:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SUBMODELS_WHEELMODE_WHEELS_YFORWARD_NOSTEER, MF_CHECKED);
		break;		
	}
}


/***************************************************************************
 * Function    : ModelConverterAddStandardLighting
 * Description : 
 ***************************************************************************/
void ModelConverterAddStandardLighting( void)
{
D3DLIGHT9 light;
D3DXVECTOR3		xVect;
ENGINE_LIGHT	xLight;
//VECT			xLightDir;

    ZeroMemory( &light, sizeof(light) );

	switch( mnLightingMode )
	{
	case 0:
	default:
		xLight.Type = DIRECTIONAL_LIGHT;
		xLight.Diffuse.r = 0.7f;
		xLight.Diffuse.g = 0.7f;
		xLight.Diffuse.b = 0.7f;
		xLight.Diffuse.a = 1.0f;
		xLight.Ambient.r = 0.4f;
		xLight.Ambient.g = 0.4f;
		xLight.Ambient.b = 0.4f;
		xLight.Ambient.a = 1.0f;
		xLight.Specular.r = 0.5f;
		xLight.Specular.g = 0.5f;
		xLight.Specular.b = 0.5f;
		xLight.Specular.a = 1.0f;
		xLight.Direction.x = -1.1f;
		xLight.Direction.y = -0.9f;
		xLight.Direction.z = -0.9f;
		VectNormalize( &xLight.Direction );
		EngineActivateLight( 0, &xLight );

		// Small amount of directional/diffuse light in roughly the opposite direction
		light.Diffuse.r = 0.4f;
		light.Diffuse.g = 0.4f;
		light.Diffuse.b = 0.4f;
		light.Ambient.r = 0.0f;
		light.Ambient.g = 0.0f;
		light.Ambient.b = 0.0f;
		light.Specular.r = 0.2f;
		light.Specular.g = 0.2f;
		light.Specular.b = 0.2f;
		
		xVect = D3DXVECTOR3( 1.2f, 0.8f, 1.0f);
		D3DXVec3Normalize( &xVect, &xVect );
		light.Direction = xVect;
			
		g_pd3dDevice->SetLight(1, &light );
		g_pd3dDevice->LightEnable(1, TRUE);
		break;
	case 1:			// Space mode.. single white light
		/** Set the main ambient light **/    
		xLight.Type = DIRECTIONAL_LIGHT;
		xLight.Diffuse.r = 1.0f;
		xLight.Diffuse.g = 1.0f;
		xLight.Diffuse.b = 1.0f;
		xLight.Diffuse.a = 1.0f;
		xLight.Ambient.r = 0.1f;
		xLight.Ambient.g = 0.1f;
		xLight.Ambient.b = 0.1f;
		xLight.Ambient.a = 1.0f;
		xLight.Specular.r = 0.5f;
		xLight.Specular.g = 0.5f;
		xLight.Specular.b = 0.5f;
		xLight.Specular.a = 1.0f;
		xLight.Direction.x = -1.1f;
		xLight.Direction.y = -0.9f;
		xLight.Direction.z = -0.2f;
		VectNormalize( &xLight.Direction );
		EngineActivateLight( 0, &xLight );
		EngineDeactivateLight( 1 );
		break;
	case 2:			// Sunset
		xLight.Type = DIRECTIONAL_LIGHT;
		xLight.Diffuse.r = 0.7f;
		xLight.Diffuse.g = 0.6f;
		xLight.Diffuse.b = 0.3f;
		xLight.Diffuse.a = 1.0f;
		xLight.Ambient.r = 0.1f;
		xLight.Ambient.g = 0.1f;
		xLight.Ambient.b = 0.1f;
		xLight.Ambient.a = 1.0f;
		xLight.Specular.r = 0.7f;
		xLight.Specular.g = 0.65f;
		xLight.Specular.b = 0.6f;
		xLight.Specular.a = 1.0f;
		xLight.Direction.x = -1.1f;
		xLight.Direction.y = -0.9f;
		xLight.Direction.z = -0.1f;
		VectNormalize( &xLight.Direction );
		EngineActivateLight( 0, &xLight );
		EngineDeactivateLight( 1 );

		light.Diffuse.r = 0.2f;
		light.Diffuse.g = 0.2f;
		light.Diffuse.b = 0.2f;
		light.Ambient.r = 0.0f;
		light.Ambient.g = 0.0f;
		light.Ambient.b = 0.0f;
		light.Specular.r = 0.0f;
		light.Specular.g = 0.0f;
		light.Specular.b = 0.0f;
		
		xVect = D3DXVECTOR3( 1.2f, 0.8f, 0.1f);
		D3DXVec3Normalize( &xVect, &xVect );
		light.Direction = xVect;
			
		g_pd3dDevice->SetLight(1, &light );
		g_pd3dDevice->LightEnable(1, TRUE);
		break;
	case 3:
		xLight.Type = DIRECTIONAL_LIGHT;
		xLight.Diffuse.r = 1.0f;
		xLight.Diffuse.g = 1.0f;
		xLight.Diffuse.b = 1.0f;
		xLight.Diffuse.a = 1.0f;
		xLight.Ambient.r = 0.1f;
		xLight.Ambient.g = 0.1f;
		xLight.Ambient.b = 0.1f;
		xLight.Ambient.a = 1.0f;
		xLight.Specular.r = 0.6f;
		xLight.Specular.g = 0.6f;
		xLight.Specular.b = 0.6f;
		xLight.Specular.a = 1.0f;
		xLight.Direction.x = 0.0f;
		xLight.Direction.y = 0.0f;
		xLight.Direction.z = -1.0f;
		VectNormalize( &xLight.Direction );
		EngineActivateLight( 0, &xLight );
		EngineDeactivateLight( 1 );
		break;
	case 4:
		xLight.Type = DIRECTIONAL_LIGHT;
		xLight.Diffuse.r = 1.0f;
		xLight.Diffuse.g = 1.0f;
		xLight.Diffuse.b = 1.0f;
		xLight.Diffuse.a = 1.0f;
		xLight.Ambient.r = 0.1f;
		xLight.Ambient.g = 0.1f;
		xLight.Ambient.b = 0.1f;
		xLight.Ambient.a = 1.0f;
		xLight.Specular.r = 0.6f;
		xLight.Specular.g = 0.6f;
		xLight.Specular.b = 0.6f;
		xLight.Specular.a = 1.0f;
		xLight.Direction.x = 0.0f;
		xLight.Direction.y = 0.0f;
		xLight.Direction.z = 1.0f;
		VectNormalize( &xLight.Direction );
		EngineActivateLight( 0, &xLight );
		EngineDeactivateLight( 1 );
		break;
	}

}

 



void	ModelConvResetCamera( void )
{
	mxBaseCamFocus.x = mxBaseCamFocus.y = mxBaseCamFocus.z = 0.0f;
	mxCamFocus.x = mxCamFocus.y = mxCamFocus.z = 0.0f;
	mxCamFocus.z = 0.05f;
	mxBaseCamFocus.z = 0.05f;
	mxCamPos.x = mxCamPos.y = mxCamPos.z = 0.0f;
	mxCamDir.x = mxCamDir.y = mxCamDir.z = 0.0f;

	mfOriginalViewAngleVert = 0.0f;
	mfViewAngleVert = 0.0f;
	mfOriginalViewAngleHoriz = 0.0f;
	mfViewAngleHoriz = 0.0f;

	mfCamDist = 10.0f / mfScaleUnit;
	mfOriginalCamDist = mfCamDist;
}

void	ModelConvResetCameraToModel( void )
{
	if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
	{
	MODEL_STATS*	pxModelStats = ModelGetStats( m_MainSceneObject.GetModelHandle() );

		ModelRecalcBounds( m_MainSceneObject.GetModelHandle() );
		mxBaseCamFocus = pxModelStats->xBoundBoxCentre;
		mxCamFocus = pxModelStats->xBoundBoxCentre;
		mxCamPos.x = mxCamPos.y = mxCamPos.z = 0.0f;
		mxCamDir.x = mxCamDir.y = mxCamDir.z = 0.0f;

		mfOriginalViewAngleVert = 0.0f;
		mfViewAngleVert = 0.0f;
		mfOriginalViewAngleHoriz = 0.0f;
		mfViewAngleHoriz = 0.0f;

		mfCamDist = (pxModelStats->fBoundSphereRadius * 2.2f) / mfScaleUnit;
		mfOriginalCamDist = mfCamDist;
	}
	else
	{
		ModelConvResetCamera();
	}

}


int ModelConvLoadModel( const char* acFilename, uint32 ulFlags, float fScale )
{
int		nLock;
int		nModelHandle;
char	acString[256];

	nLock = ModelGetLockState( acFilename );
	switch ( nLock )
	{
	default:
#ifdef	MODEL_CONV_NO_LOCK_PROTECTED 
	case 1:
	case 2:
		ModelConvTextAdd( "Model Locked - Loading anyway.." );
#else
	case 1:
		ModelConvTextAdd( "Model Locked - This model can only be used on registered islands." );
		return(NOTFOUND);
		break;
	case 2: // The model is locked and can be used on a specific island only
		ModelConvTextAdd( "This model can only be accessed on the computer it was built for. );
		return(NOTFOUND);
		break;
#endif
	case 0:
		nModelHandle = ModelLoad( acFilename, 0, 1.0f );
		sprintf( acString, "Model Converter %s - %s", MODELCONV_VERSIONSTRING, acFilename );
		SendMessage( mhwndMainDialog, WM_SETTEXT, 0, (LPARAM)acString );
		break;
	}
	if ( nModelHandle != NOTFOUND )
	{
		mnCurrentWheelMode = maxModelRenderData[nModelHandle].bWheelDisplayMode;
		ModelConvSetWheelModeOption( mnCurrentWheelMode );
	}

	return( nModelHandle );
}


const char*	ModelConvGetCurrentRenderTexture( void )
{
	if ( macLoadedTextureFilename[0] != 0 )
	{
		return( macLoadedTextureFilename );
	}
	return( NULL );
}

int ModelConvGetCurrentModel( void )
{
	return( m_MainSceneObject.GetModelHandle() );
}


void ModelConvSetCurrentModel( int nHandle )
{
	if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
	{
		ModelFree( m_MainSceneObject.GetModelHandle() );
	}
	m_MainSceneObject.SetModel( nHandle );
	m_MainSceneObject.OnModelChanged( FALSE );
}

void ModelConvSelectByMaterial( int attribID )
{
	m_MainSceneObject.SelectFacesByMaterial(attribID);
}


void	ModelConvSetTemporaryDisplayModel( int nHandle )
{
	mnTemporaryDisplayModelHandle = nHandle;
}

//---------------------------------------------------
// Function : ModelConvSetExportFlags
//		
//---------------------------------------------------
void	ModelConvSetExportFlags( uint32 ulFlag )
{
	if ( (ulFlag &ATM_EXPORT_FLAGS_COMPRESSION) )
	{	
		CheckMenuItem(GetMenu(mhwndMainDialog), IDM_ATM_COMPRESSION, MF_CHECKED);
	}
	else
	{
		CheckMenuItem(GetMenu(mhwndMainDialog), IDM_ATM_COMPRESSION, MF_UNCHECKED);
	}
	if ( (ulFlag &ATM_EXPORT_FLAGS_MATERIALS) )
	{	
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_EXPORTOPTIONS_MATERIALS, MF_CHECKED);
	}
	else
	{
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_EXPORTOPTIONS_MATERIALS, MF_UNCHECKED);
	}
	if ( (ulFlag &ATM_EXPORT_FLAGS_VERY_COMPRESSED_UVS) )
	{	
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_EXPORTOPTIONS_COMPRESSTEXTURECOORDS, MF_CHECKED);
	}
	else
	{
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_EXPORTOPTIONS_COMPRESSTEXTURECOORDS, MF_UNCHECKED);
	}

	if ( (ulFlag &ATM_EXPORT_FLAGS_COLOURS) )
	{	
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_EXPORTOPTIONS_EXPORTVERTEXCOLOURS, MF_CHECKED);
	}
	else
	{
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_EXPORTOPTIONS_EXPORTVERTEXCOLOURS, MF_UNCHECKED);

	}
	if ( (ulFlag &ATM_EXPORT_FLAGS_COMPRESSED_VERTICES) )
	{	
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_TOOLS_COMPRESSVERTICES, MF_CHECKED);
	}
	else
	{
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_TOOLS_COMPRESSVERTICES, MF_UNCHECKED);
	}
}


void		ModelConvSelectionWindowReset( void )
{
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SELECTION_GROUP ), SW_HIDE );
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_ASSIGN_MATERIAL ), SW_HIDE );
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_UVUNWRAP ), SW_HIDE );	
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SUBDIVIDE ), SW_HIDE );	
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SELECTIONTEXT ), SW_HIDE );	
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_DELETE_FACES ), SW_HIDE );	
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SELECT_CONNECTED ), SW_HIDE );	

	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SLICE_FACES ), SW_HIDE );	
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_AXIS_SNAP ), SW_HIDE );	
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_RESET_PLANE ), SW_HIDE );	

	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SELECT_VERTS ), SW_HIDE );	
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_MOVE_VERTS ), SW_HIDE );	
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_ROTATE_VERTS ), SW_HIDE );	
	ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SCALE_VERTS ), SW_HIDE );	
}

void	ModelConvSetVertexSelectionMode( int nMode )
{
	SendDlgItemMessage( mhwndMainDialog, IDC_SELECT_VERTS, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndMainDialog, IDC_MOVE_VERTS, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndMainDialog, IDC_ROTATE_VERTS, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndMainDialog, IDC_SCALE_VERTS, BM_SETCHECK, BST_UNCHECKED, 0 );

	switch( nMode )
	{
	case 0:
		SendDlgItemMessage( mhwndMainDialog, IDC_SELECT_VERTS, BM_SETCHECK, BST_CHECKED, 0 );
		mnPickerControlMode = CONTROL_VERTEXPICKER;
		break;
	case 1:
		SendDlgItemMessage( mhwndMainDialog, IDC_MOVE_VERTS, BM_SETCHECK, BST_CHECKED, 0 );
		mnPickerControlMode = CONTROL_VERTEXMOVE;
		VertexManipulateSetControlMode( VERTEX_MANIPULATE_MOVE );
		break;
	case 2:
		SendDlgItemMessage( mhwndMainDialog, IDC_ROTATE_VERTS, BM_SETCHECK, BST_CHECKED, 0 );
		mnPickerControlMode = CONTROL_VERTEXROTATE;
		VertexManipulateSetControlMode( VERTEX_MANIPULATE_ROTATE );
		break;
	case 3:
		SendDlgItemMessage( mhwndMainDialog, IDC_SCALE_VERTS, BM_SETCHECK, BST_CHECKED, 0 );
		mnPickerControlMode = CONTROL_VERTEXSCALE;
		VertexManipulateSetControlMode( VERTEX_MANIPULATE_SCALE );
		break;
	}
}



void ModelConvShowSelectionWindow( int mode )
{
	switch( mode ) 
	{ 
	case 0:		// Selection options off
		ModelConvSelectionWindowReset();
		break;
	case 1:		// Face selection options
		ModelConvSelectionWindowReset();
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SELECTION_GROUP ), SW_SHOW );
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_ASSIGN_MATERIAL ), SW_SHOW );
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_UVUNWRAP ), SW_SHOW );	
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SUBDIVIDE ), SW_SHOW );	
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SELECTIONTEXT ), SW_SHOW );	
		SetDlgItemText( mhwndMainDialog, IDC_SELECTIONTEXT, (LPCSTR)"Face Selection" );
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_DELETE_FACES ), SW_SHOW );	
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SELECT_CONNECTED ), SW_SHOW );	
		break;
	case 2:		// Poly slice options
		ModelConvSelectionWindowReset();
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SELECTIONTEXT ), SW_SHOW );	
		SetDlgItemText( mhwndMainDialog, IDC_SELECTIONTEXT, (LPCSTR)"Poly Slice" );
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SLICE_FACES ), SW_SHOW );	
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_AXIS_SNAP ), SW_SHOW );	
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_RESET_PLANE ), SW_SHOW );	
		break;
	case 3:		// Vertex select options
		ModelConvSelectionWindowReset();
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SELECTIONTEXT ), SW_SHOW );	
		SetDlgItemText( mhwndMainDialog, IDC_SELECTIONTEXT, (LPCSTR)"Vertex Selection" );
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SELECT_VERTS ), SW_SHOW );	
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_MOVE_VERTS ), SW_SHOW );	
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_ROTATE_VERTS ), SW_SHOW );	
		ShowWindow( GetDlgItem( mhwndMainDialog,IDC_SCALE_VERTS ), SW_SHOW );	
		ModelConvSetVertexSelectionMode( 0 );
		break;
	}

}


void	ModelConvSetLODUsedDisplay( int nLODUsed )
{
	mnLODUsedDisplay = nLODUsed;
	
	switch ( mnLODUsedDisplay )
	{
	case 0: 
	default:
		SetDlgItemText( mhwndMainDialog, IDC_LODUSED_TEXT, "(Current LOD : High)" );
		break;
	case 1:
		SetDlgItemText( mhwndMainDialog, IDC_LODUSED_TEXT, "(Current LOD : Med)" );
		break;
	case 2:
		SetDlgItemText( mhwndMainDialog, IDC_LODUSED_TEXT, "(Current LOD : Low)" );
		break;
	}
}
//---------------------------------------------------
// Function : ModelConvGetExportFlags
//		
//---------------------------------------------------
uint32	ModelConvGetExportFlags( void )
{
uint32	ulFlags = 0;


	if ( (GetMenuState( GetMenu(mhwndMainDialog), IDM_ATM_COMPRESSION, 0 ) & MF_CHECKED) != 0 )
	{
		ulFlags |= ATM_EXPORT_FLAGS_COMPRESSION;
	}
	if ( (GetMenuState( GetMenu(mhwndMainDialog), ID_EXPORTOPTIONS_MATERIALS, 0 ) & MF_CHECKED) != 0 )
	{
		ulFlags |= ATM_EXPORT_FLAGS_MATERIALS;
	}
	if ( (GetMenuState( GetMenu(mhwndMainDialog), ID_EXPORTOPTIONS_COMPRESSTEXTURECOORDS, 0 ) & MF_CHECKED) != 0 )
	{
		ulFlags |= ATM_EXPORT_FLAGS_VERY_COMPRESSED_UVS;
	}
	if ( (GetMenuState( GetMenu(mhwndMainDialog), ID_EXPORTOPTIONS_EXPORTVERTEXCOLOURS, 0 ) & MF_CHECKED) != 0 )
	{
		ulFlags |= ATM_EXPORT_FLAGS_COLOURS;
	}
	if ( (GetMenuState( GetMenu(mhwndMainDialog), ID_TOOLS_COMPRESSVERTICES, 0 ) & MF_CHECKED) != 0 )
	{
		ulFlags |= ATM_EXPORT_FLAGS_COMPRESSED_VERTICES;
	}
	return( ulFlags );
}


/***************************************************************************
 * Function    : ModelConverterDisplayFrame
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void	ModelConverterDrawFloor( void )
{
MVECT		xPos1;
MVECT		xPos2;
int			nLoop;
int			nSquares = 11;
int			nMidSquare = 5;

	if ( mnScaleUnitMode == 0 )		// theuniversal scaling mdoe
	{
		mfFloorSize = SIZE_OF_ONE_TILE * 5.0f;
		mfFloorSteps = SIZE_OF_ONE_TILE;
	}
	else
	{
		mfFloorSize = 10.0f;
		mfFloorSteps = 1.0f;
		nSquares = 21;
		nMidSquare = 10;
	}

	xPos1.x = -mfFloorSize;
	xPos1.y = -mfFloorSize;
	xPos1.z = 0.0f;

	xPos2.x = -mfFloorSize;
	xPos2.y = mfFloorSize;
	xPos2.z = 0.0f;

	for ( nLoop = 0; nLoop < nSquares; nLoop++ )
	{
		if ( nLoop == nMidSquare )
		{
			AddLine( &xPos1, &xPos2, 0xD03030A0, 0xD03030A0 );
		}
		else
		{
			AddLine( &xPos1, &xPos2, 0xA0101010, 0xA0101010 );
		}
		xPos1.x += mfFloorSteps;
		xPos2.x += mfFloorSteps;
	}

	xPos1.x = -mfFloorSize;
	xPos1.y = -mfFloorSize;
	xPos1.z = 0.0f;
	xPos2.x = mfFloorSize;
	xPos2.y = -mfFloorSize;
	xPos2.z = 0.0f;

	for ( nLoop = 0; nLoop < nSquares; nLoop++ )
	{
		if ( nLoop == nMidSquare )
		{
			AddLine( &xPos1, &xPos2, 0xD04040B0, 0xD04040B0 );
		}
		else
		{
			AddLine( &xPos1, &xPos2, 0xA0303030, 0xA0303030);
		}
		xPos1.y += mfFloorSteps;
		xPos2.y += mfFloorSteps;
	}
}


/***************************************************************************
 * Function    : ModelConverterSetupCamera
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void		ModelConverterSetupCamera( void )
{
D3DXMATRIX matProj;
D3DXMATRIX matWorld;
D3DXMATRIX matView;
D3DXMATRIX matRot1;
D3DXMATRIX matRot2;
float	fNearClipPlane = 0.01f;
float	fFarClipPlane = 200.0f;
float	fFOV = (float)(PI/4);
VECT	xCamUp;

	if ( mfScaleUnit == 1.0f )
	{
		fNearClipPlane = 0.01f;
		fFarClipPlane = 10000.0f;
	}
	mxCamDir.x = 0.0f;
	mxCamDir.y = -1.0f;
	mxCamDir.z = 0.0f;
	xCamUp.x = 0.0f;
	xCamUp.y = 0.0f;
	xCamUp.z = 1.0f;
	switch( mnViewPage )
	{
	case 0:		// 3d view
		VectRotateAboutX( &mxCamDir, mfViewAngleVert );
		VectRotateAboutZ( &mxCamDir, mfViewAngleHoriz );
		VectRotateAboutX( &xCamUp, mfViewAngleVert );
		VectRotateAboutZ( &xCamUp, mfViewAngleHoriz );
		break;
	case 1:		// Left
		mxCamDir.x = 1.0f;
		mxCamDir.y = 0.0f;
		mxCamDir.z = 0.0f;
		break;
	case 2:		// Front
		mxCamDir.x = 0.0f;
		mxCamDir.y = -1.0f;
		mxCamDir.z = 0.0f;
		break;
	case 3:		// Right
		mxCamDir.x = -1.0f;
		mxCamDir.y = 0.0f;
		mxCamDir.z = 0.0f;
		break;
	case 4:		// Back
		mxCamDir.x = 0.0f;
		mxCamDir.y = 1.0f;
		mxCamDir.z = 0.0f;
		break;
	case 5:		// top
		mxCamDir.x = 0.0f;
		mxCamDir.y = 0.0f;
		mxCamDir.z = -1.0f;
		xCamUp.x = 0.0f;
		xCamUp.y = -1.0f;
		xCamUp.z = 0.0f;
		break;
	case 6:		// bottom
		mxCamDir.x = 0.0f;
		mxCamDir.y = 0.0f;
		mxCamDir.z = 1.0f;
		xCamUp.x = 0.0f;
		xCamUp.y = -1.0f;
		xCamUp.z = 0.0f;
		break;
	}
   
	mxCamPos.x = mxCamFocus.x + ( (mxCamDir.x * mfCamDist ) * -1.0f );
	mxCamPos.y = mxCamFocus.y + ( (mxCamDir.y * mfCamDist ) * -1.0f );
	mxCamPos.z = mxCamFocus.z + ( (mxCamDir.z * mfCamDist ) * -1.0f );

	CameraSetPosIgnoreShake( mxCamPos.x, mxCamPos.y, mxCamPos.z );
	EngineCameraSetPos( mxCamPos.x, mxCamPos.y, mxCamPos.z );
	EngineCameraSetUpVect( xCamUp.x,xCamUp.y, xCamUp.z );
	EngineCameraSetDirection( mxCamDir.x, mxCamDir.y, mxCamDir.z );

	EngineCameraUpdate();
/*
	// Set view matrix at camera position looking at 0,0,0
    D3DXMatrixIdentity( &matView );
	D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3( mxCamPos.x, mxCamPos.y, mxCamPos.z ),
		                          &D3DXVECTOR3( mxCamFocus.x, mxCamFocus.y, mxCamFocus.z ),
			                      &D3DXVECTOR3( 0.0f, 0.0f, 1.0f ) );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	// Set the world matrix as identity
	D3DXMatrixIdentity( &matWorld );	
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
*/		
	// Set the projection matrix
	float	fAspect = (float)(InterfaceGetWidth()) / (float)(InterfaceGetHeight());
	D3DXMatrixPerspectiveFovLH( &matProj, fFOV, fAspect, fNearClipPlane, fFarClipPlane );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

}

void	ModelConvNewFrame( void )
{
uint32		ulCol;

	// If we need to change and we currently have a d3d device
	if ( ( InterfaceDoesNeedChanging() == TRUE ) &&
		 ( InterfaceGetD3DDevice() != NULL ) )
	{
		// Free all graphics
//		GameFreeAllGraphics();
		PlaneCutterFreeGraphics();

		EngineFree( FALSE );
		FreePointRenderer();
		FreeLineRenderer();
		Sprites3DShutdown();

		m_MainSceneObject.ReleaseDX();
		mboIsResizing = TRUE;
	}
	switch( mnBackgroundColour )
	{
	case 0:
		ulCol = 0xFF606060;
		break;
	case 1:
		ulCol = 0xFF00FF00;
		break;
	case 2:
		ulCol = 0xFF000000;
		break;
	case 3:
		ulCol = 0xFFFFFFFF;
		break;
	case 4:
		ulCol = 0xFF0000FF;
		break;
	case 5:
		ulCol = 0xFFFF0000;
		break;
	}
	switch(	InterfaceNewFrame( ulCol ) )
	{
	case 0:
		break;
	case 1:
		g_pd3dDevice = InterfaceGetD3DDevice();
//		InterfaceGetD3D();
		InterfaceInit( FALSE );
		EngineRestart();
		InitialiseLineRenderer(32768);
		InitialisePointRenderer();
		Sprites3DInitialise();
		m_MainSceneObject.InitDX();
		PlaneCutterInitGraphics();
		ModelConverterAddStandardLighting();
		mboIsResizing = FALSE;
		break;
	default:
		PostQuitMessage(0);
		break;
	}
}


/***************************************************************************
 * Function    : ModelConverterUpdateDisplayStats
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void		ModelConverterUpdateDisplayStats( void )
{
	if ( mnLastDisplayPolyRendered != mnPolysRendered )
	{
	char	acBuff[256];
		mnLastDisplayPolyRendered = mnPolysRendered;
		sprintf( acBuff, "%d polys", mnLastDisplayPolyRendered );
		SetDlgItemText( mhwndMainDialog, IDC_DISPLAY_TEXT2, acBuff );
	}
}

void	ModelConvUpdated3dPointerCoord( void )
{
char acString[256] = "";
VECT	xRayStart;
VECT	xRayDir;
VECT	xRayEnd;
VECT	xHit;
VECT	xHitNormal;
VECT	xPos = { 0.0f, 0.0f, 0.0f };
VECT	xRot = { 0.0f, 0.0f, 0.0f };
int		nFaceHit = 0;

	m_sModelConvViewInterface.GetRayForScreenCoord( (int)mxMousePos.x, (int)mxMousePos.y, &xRayStart, &xRayDir );
	xRayEnd = xRayDir;
	VectNormalize( &xRayEnd );
	VectScale( &xRayEnd, &xRayEnd, 100000.0f );
	VectAdd( &xRayEnd, &xRayEnd, &xRayStart );
	
	if ( ModelRayTest( m_MainSceneObject.GetModelHandle(), &xPos, &xRot, &xRayStart, &xRayEnd, &xHit, &xHitNormal, &nFaceHit, 0 ) == TRUE )
	{
		sprintf( acString, "%.5f", xHit.x );
		SetDlgItemText( mhwndMainDialog, IDC_POSVALUEX, acString );
		sprintf( acString, "%.5f", xHit.y );
		SetDlgItemText( mhwndMainDialog, IDC_POSVALUEY, acString );
		sprintf( acString, "%.5f", xHit.z );
		SetDlgItemText( mhwndMainDialog, IDC_POSVALUEZ, acString );
	}
}

void		ModelConverterDisableButtons( void )
{
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_VIEWLODTEXT ), FALSE );
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_MAINLODCOMBO ), FALSE );

	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIMTEXT ), FALSE );
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_COMBO1 ), FALSE );
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIM_STOP ), FALSE );
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIM_PLAY ), FALSE );
	
	
}

void		ModelConverterUpdateButtonStates( MODEL_RENDER_DATA* pxModelData )
{
	if ( pxModelData->xStats.bNumLODs >= 1 )
	{
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_VIEWLODTEXT ), TRUE );
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_MAINLODCOMBO ), TRUE );
	}
	else
	{
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_VIEWLODTEXT ), FALSE );
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_MAINLODCOMBO ), FALSE );
	}

	if ( pxModelData->pxVertexKeyframes )
	{
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIMTEXT ), TRUE );
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_COMBO1 ), TRUE );
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIM_STOP ), TRUE );
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIM_PLAY ), TRUE );
	}
	else
	{
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIMTEXT ), FALSE );
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_COMBO1 ), FALSE );
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIM_STOP ), FALSE );
		EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIM_PLAY ), FALSE );
	}

}


void		ModelConverterRenderShadowMapDepthPass( void )
{
	if ( EngineSceneShadowMapIsActive() == TRUE )
	{
	int		nHandleToDraw;

		nHandleToDraw = m_MainSceneObject.GetModelHandle();

		if ( nHandleToDraw != NOTFOUND )
		{
		VECT	xPos = { 0.0f, 0.0f, 0.0f };
		float	fLightCamTargetDist = 45.0f * mfCamDist * 0.12f;
		float	fLightNearPlane = mfCamDist * 0.14f;
		float	fLightFarPlane = mfCamDist * 11.0f;
		VECT	xCamTarget = { 0.0f, 0.0f, 0.0f };
		VECT	xCamPos = *EngineCameraGetPos();

//			EngineSceneShadowsSetRenderParams( fLightCamTargetDist * 1.0f, fLightNearPlane * 10.0f, fLightFarPlane * 1.0f, 0.14f );
			EngineSceneShadowsSetRenderParams( 50.0f, 10.0f, 70.0f, 0.01f );
			EngineSceneShadowsSetShaderEpsilon( 0.004f );

			EngineSceneShadowsInitShadowPass( &xCamTarget );
			EngineEnableZWrite( TRUE );
			EngineEnableZTest( TRUE );
			EngineEnableBlend( FALSE );

			ModelRender( nHandleToDraw, &xPos, NULL, RENDER_FLAGS_DRAWMESH_ONLY );

			EngineSceneShadowsEndShadowPass();
		}
	}
}

void	ModelConverterDrawAxes( void )
{
VECT	xRayStart;
VECT	xRayDir;
VECT	xAxis1;
int		nScreenX, nScreenY;

	EngineGetRayForScreenCoord( 70, InterfaceGetHeight() - 70, &xRayStart, &xRayDir );
	VectScale( &xRayDir, &xRayDir, 100.0f );

	VectAdd( &xRayStart, &xRayStart, &xRayDir );

	xAxis1.x = 7.0f;
	xAxis1.y = 0.0f;
	xAxis1.z = 0.0f;
	VectAdd( &xAxis1, &xRayStart, &xAxis1 );

	AddLine( &xRayStart, &xAxis1, 0xffF0C050, 0xD0F0C050 );
	xRayStart.y -= 0.02f;
	xRayStart.z -= 0.03f;
	AddLine( &xRayStart, &xAxis1, 0xffF0C050, 0xD0F0C050 );

	EngineGetScreenCoordForWorldCoord( &xAxis1, &nScreenX, &nScreenY );	
	InterfaceText( 0, nScreenX - 5, nScreenY - 15, "X", 0xD0F0C050, 0 );

	xAxis1.x = 0.0f;
	xAxis1.y = 7.0f;
	xAxis1.z = 0.0f;
	VectAdd( &xAxis1, &xRayStart, &xAxis1 );

	AddLine( &xRayStart, &xAxis1, 0xff50B050, 0xD050B050 );
	xRayStart.y -= 0.02f;
	xRayStart.z -= 0.03f;
	AddLine( &xRayStart, &xAxis1, 0xff50B050, 0xD050B050 );

	EngineGetScreenCoordForWorldCoord( &xAxis1, &nScreenX, &nScreenY );	
	InterfaceText( 0, nScreenX - 5, nScreenY - 15, "Y", 0xD050B050, 0 );

	xAxis1.x = 0.0f;
	xAxis1.y = 0.0f;
	xAxis1.z = 7.0f;
	VectAdd( &xAxis1, &xRayStart, &xAxis1 );

	AddLine( &xRayStart, &xAxis1, 0xff4040A0, 0xD04040A0 );
	xRayStart.y -= 0.02f;
	xRayStart.z -= 0.03f;
	AddLine( &xRayStart, &xAxis1, 0xff4040A0, 0xD04040A0 );

	EngineGetScreenCoordForWorldCoord( &xAxis1, &nScreenX, &nScreenY );	
	InterfaceText( 0, nScreenX - 5, nScreenY - 15, "Z", 0xD04040A0, 0 );
}


void		ModelConverterKeyControlsUpdate( uint32 ulTick )
{
	if ( ( mbThisWindowIsFocused ) &&
		 ( mbDisableWASD == FALSE ) ) 
	{
	float		fDelta = ((float)ulTick) / 1000.0f;
	float		fMoveAmount = 1.0f;
	VECT		xCamDir = *EngineCameraGetDirection();
	VECT		xCamUp = *EngineCameraGetUpVect();
	VECT		xCamRight;
	VECT		xMove;
	float		fMinDist;

		VectNormalize( &xCamDir );
		VectNormalize( &xCamUp );
		VectCross( &xCamRight, &xCamUp, &xCamDir );

		if ( SysCheckKeyState( KEY_SHIFT ) == TRUE )
		{
			fMoveAmount = 3.0f;
		}
		fMinDist = 3.0f + (fDelta * fMoveAmount);
	
		if ( SysCheckKeyState( KEY_WASD_UP ) == TRUE )
		{
			if ( mfCamDist > fMinDist )
			{
				mfCamDist -= fDelta * fMoveAmount;
			}
			else
			{
				VectScale( &xMove, &xCamDir, fDelta * fMoveAmount );
				VectAdd( &mxCamFocus, &mxCamFocus, &xMove );
			}
		}
		else if ( SysCheckKeyState( KEY_WASD_DOWN ) == TRUE )
		{
			if ( mfCamDist < 15.0f )
			{
				mfCamDist += fDelta * fMoveAmount;
			}
			else
			{
				VectScale( &xMove, &xCamDir, fDelta * fMoveAmount * -1.0f );
				VectAdd( &mxCamFocus, &mxCamFocus, &xMove );
			}
		}

		if ( SysCheckKeyState( KEY_WASD_LEFT ) == TRUE )
		{
			VectScale( &xMove, &xCamRight, fDelta * fMoveAmount * -1.0f );
			VectAdd( &mxCamFocus, &mxCamFocus, &xMove );
		}
		else if ( SysCheckKeyState( KEY_WASD_RIGHT ) == TRUE )
		{
			VectScale( &xMove, &xCamRight, fDelta * fMoveAmount );
			VectAdd( &mxCamFocus, &mxCamFocus, &xMove );
		}

		if ( SysCheckKeyState( KEY_WASD_ACTION1 ) == TRUE )
		{
			VectScale( &xMove, &xCamUp, fDelta * fMoveAmount );
			VectAdd( &mxCamFocus, &mxCamFocus, &xMove );
		}
		else if ( SysCheckKeyState( KEY_WASD_ACTION2 ) == TRUE )
		{
			VectScale( &xMove, &xCamUp, fDelta * fMoveAmount * -1.0f );
			VectAdd( &mxCamFocus, &mxCamFocus, &xMove );
		}
	}
}

/***************************************************************************
 * Function    : ModelConverterDisplayFrame
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void		ModelConverterDisplayFrame( BOOL bModelHasChanged )
{
int		nHandleToDraw;
MODEL_RENDER_DATA*		pxModelData;
VECT	xPos;
VECT*	pxAttachPos;
uint32	ulTickGap;
bool	bDrawUsingSceneObject = true;
float	fDelta;

	if ( mboIsResizing == TRUE )
	{
		return;
	}

	if ( bModelHasChanged )
	{
		m_MainSceneObject.OnModelChanged( FALSE );
	}

	
	ulTickGap = GetTickCount()-gulLastTick;
	if ( ulTickGap > 100 ) ulTickGap = 100;
	ModelConverterKeyControlsUpdate( ulTickGap );
	ModelRenderingUpdate( ulTickGap / 10 );
	ModelConvUpdated3dPointerCoord();
	EngineUpdate(TRUE);

	fDelta = (float)( ulTickGap * 0.001f );

	mfWheelTurnRot += (float)( (PI*0.001f) * (float)( ulTickGap ) );
	if ( mfWheelTurnRot > TwoPi )
	{
		mfWheelTurnRot -= TwoPi;
	}
	RenderEffectsUpdate( fDelta );

	gulLastTick = GetTickCount();

	ModelConverterSetupCamera();

	ModelConvNewFrame();

	if ( InterfaceDoesNeedChanging() == TRUE ) return;

	InterfaceBeginRender();

	ModelConverterSetupCamera();

	InterfaceSetFontFlags(FONT_FLAG_SMALL);
	InterfaceText( 1, 10, 10, "A Tractor Model Converter", COL_COMMS_SPECIAL, 0 );
	InterfaceSetFontFlags(0);

	ModelConverterRenderShadowMapDepthPass();

	ModelConverterSetupCamera();

	if ( mboDrawGridLines == TRUE )
	{
		ModelConverterDrawFloor();
	}

	ModelConverterDrawAxes();

	nHandleToDraw = m_MainSceneObject.GetModelHandle();

	if ( mnTemporaryDisplayModelHandle != NOTFOUND )
	{
		nHandleToDraw = mnTemporaryDisplayModelHandle;
		bDrawUsingSceneObject = false;
	}
		
	if ( nHandleToDraw  != NOTFOUND )
	{	
	int		nVal;
//		ModelRenderSetWheelDisplayMode(nHandleToDraw, 1, mfWheelTurnRot );
		ModelSetWheelSpinRotation( nHandleToDraw, TwoPi-mfWheelTurnRot );
		ModelRenderSetWheelDisplayMode( nHandleToDraw, mnCurrentWheelMode, mfWheelTurnRot );

		pxModelData = &maxModelRenderData[ nHandleToDraw ];

		EngineSetTexture( 0, m_MainSceneObject.GetTextureHandle() );

		if ( pxModelData->xCollisionAttachData.nModelHandle != NOTFOUND )
		{
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_SHOW_COLLISION ), TRUE );
			nVal = SendDlgItemMessage( mhwndMainDialog, IDC_SHOW_COLLISION, BM_GETCHECK, 0, 0 );
			if ( nVal == BST_CHECKED )
			{
				nHandleToDraw = pxModelData->xCollisionAttachData.nModelHandle;
				pxModelData = &maxModelRenderData[ nHandleToDraw ];
				g_pd3dDevice->SetTexture( 0, NULL );
			}
		}
		else
		{
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_SHOW_COLLISION ), FALSE );
		}

		ModelConverterUpdateButtonStates( pxModelData );

		g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
		EngineEnableBlend( TRUE );
		EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
		EngineEnableAlphaTest( TRUE );
		EngineEnableZWrite( TRUE );
		EngineEnableZTest( TRUE );
		EngineEnableFog( FALSE );
		EngineEnableCulling( 1 );
		EngineEnableLighting( TRUE );
		EngineEnableSpecular(TRUE);
		EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );

		xPos.x = 0.0f;
		xPos.y = 0.0f;
		xPos.z = 0.0f;
		if ( bDrawUsingSceneObject )
		{
		int		nRenderingMode = mnModelRenderingMode;

			// If using picker control, always show face edges etc
			switch( mnPickerControlMode )
			{
			case CONTROL_PICKER:
			case CONTROL_POLYSLICE:
				nRenderingMode = 1;
				break;
			case CONTROL_VERTEXPICKER:
			case CONTROL_VERTEXMOVE:
			case CONTROL_VERTEXROTATE:
			case CONTROL_VERTEXSCALE:
				nRenderingMode = 2;
				break;
			}

			if ( EngineSceneShadowMapIsActive() )
			{
				EngineSceneShadowsStartRender( TRUE, FALSE, FALSE );
			}
			else
			{
				EngineSetShadowMultitexture( FALSE );
			}

			switch( nRenderingMode )
			{
			case 0:
				if ( mboShowNormals )
				{
					mnPolysRendered = m_MainSceneObject.Render(CSceneObject::NORMAL_SHOWNORMALS);
				}
				else
				{
					mnPolysRendered = m_MainSceneObject.Render(CSceneObject::NORMAL);
				}
				break;
			case 1:
				if ( mboShowNormals )
				{
					mnPolysRendered = m_MainSceneObject.Render(CSceneObject::NORMAL_WIREFRAME_FACES_WITH_SHOWNORMALS);
				}
				else
				{
					mnPolysRendered = m_MainSceneObject.Render(CSceneObject::NORMAL_WIREFRAME_FACES);
				}
				break;
			case 2:
				mnPolysRendered = m_MainSceneObject.Render(CSceneObject::NORMAL_WITH_VERTEX_POINTS);
				break;
			}

			if ( EngineSceneShadowMapIsActive() )
			{
				EngineSceneShadowsEndRender();
			}
		}
		else
		{
			mnPolysRendered = ModelRender( nHandleToDraw, &xPos, NULL, 0 );

			if ( ModelShadowsEnabled( nHandleToDraw ) )
			{
				//-------------------------------------------------------------------
				// TEMP - For testing only!!!!!!!!!!!!!!
				ShadowVolumeRenderModel( nHandleToDraw, &xPos, NULL, 0, 0 );
				//-------------------------------------------------------------------
			}
		}
		
		if ( AttachDlgBallAttachPoint() != 0 )
		{
			pxAttachPos = ModelGetAttachPoint( nHandleToDraw, AttachDlgBallAttachPoint() );
			if ( pxAttachPos != NULL )
			{
				EngineSetTexture( 0, NOTFOUND );
				ModelRender( mnBallModelHandle, pxAttachPos, NULL, 0 );
			}
		}

		if ( ModelGetStats(nHandleToDraw)->bLODUsed != mnLODUsedDisplay )
		{
			ModelConvSetLODUsedDisplay( ModelGetStats(nHandleToDraw)->bLODUsed );
		}
	}
	else		// No model to draw
	{
		ModelConverterDisableButtons();
	}

	PlaneCutterRender();
	RenderEffectsRender();
	Sprites3DFlush( FALSE );
	RenderPoints( 4.0f, FALSE, FALSE );
	RenderLines( FALSE,FALSE, FALSE, TRUE );

	if ( mbModelConvShowSelectionRect )
	{
	int	x = (int)mxMouseDownPos.x;
	int	y = (int)mxMouseDownPos.y;
	int x2 = (int)mxMousePos.x;
	int y2 = (int)mxMousePos.y;

		InterfaceLine( 0, x, y, x, y2, 0xD0F0E060, 0xD0F0E060 );
		InterfaceLine( 0, x, y, x2, y, 0xD0F0E060, 0xD0F0E060 );
		InterfaceLine( 0, x2, y, x2, y2, 0xD0F0E060, 0xD0F0E060 );
		InterfaceLine( 0, x, y2, x2, y2, 0xD0F0E060, 0xD0F0E060 );
	}

	//-------------------------------------------------------------------
	// TEMP - For testing only!!!!!!!!!!!!!!
//	ShadowVolumeRender();
//	ShadowVolumeUpdate();
	//-------------------------------------------------------------------

	InterfaceDraw();

	InterfaceEndRender();
	InterfacePresent();

	ModelConverterUpdateDisplayStats();

}






void	ModelConvRotateModelFree( int nHandle, float fX, float fY, float fZ, BOOL bAffectSubModels )
{
	if ( nHandle != NOTFOUND )
	{
		ModelRotate( nHandle, fX, fY, fZ, bAffectSubModels );
	}
	ModelConverterDisplayFrame( TRUE );

}

void	ModelConvRotate90( int nHandle )
{
	if ( nHandle != NOTFOUND )
	{
		ModelRotate( nHandle, 0.0f, 0.0f, A90, TRUE );
	}
	ModelConverterDisplayFrame( TRUE );

}


void ModelConvMidMouseDownMove( float fScreenXDelta, float fScreenYDelta )
{
D3DXVECTOR3		xVect;
D3DXMATRIX	matRotX;
D3DXMATRIX	matRotZ;
float	fMoveSpeed = 0.004f;

	switch( mnPickerControlMode )
	{
	case CONTROL_CAMERA:
	case CONTROL_PICKER:
	case CONTROL_VERTEXPICKER:
	case CONTROL_VERTEXMOVE:
	case CONTROL_VERTEXROTATE:
	case CONTROL_VERTEXSCALE:
	case CONTROL_POLYSLICE:
	default:
		mxCamFocus.z = mxBaseCamFocus.z + (fScreenYDelta*fMoveSpeed);		
		xVect.x = 1.0f;
		xVect.y = 0.0f;
		xVect.z = 0.0f;
		D3DXMatrixRotationZ( &matRotZ, mfOriginalViewAngleHoriz );
		D3DXVec3TransformCoord( &xVect, &xVect, &matRotZ );
		mxCamFocus.x = mxBaseCamFocus.x - (fScreenXDelta*xVect.x*fMoveSpeed);
		mxCamFocus.y = mxBaseCamFocus.y - (fScreenXDelta*xVect.y*fMoveSpeed);
		break;
	}
//	LevelEdCamSetTransScreenDelta( fScreenXDelta, fScreenYDelta );
}


void ModelConvMouseDownStore( void )
{
	mfOriginalCamDist = mfCamDist;
	mfOriginalViewAngleVert = mfViewAngleVert;
	mfOriginalViewAngleHoriz = mfViewAngleHoriz;
	mxBaseCamFocus = mxCamFocus;

	PlaneCutterMouseDownStore();
}

void	ModelConvSetZoomScreenDelta( float fOffset, float fScale )
{
	mfCamDist += (fOffset * fScale * 0.02f);
	if ( mfCamDist < 0.001f )
	{
		mfCamDist = 0.001f;
	}
	ModelConverterSetupCamera();
	ModelConverterDisplayFrame( FALSE );
}

void ModelConvRightMouseDownMove( float fScreenXDelta, float fScreenYDelta )
{
D3DXVECTOR3		xVect;
D3DXMATRIX	matRotX;
D3DXMATRIX	matRotZ;

	switch( mnPickerControlMode )
	{
	case CONTROL_MOVE:
		mxCamFocus.z = mxBaseCamFocus.z + (fScreenYDelta*0.01f);		
		xVect.x = 1.0f;
		xVect.y = 0.0f;
		xVect.z = 0.0f;
		D3DXMatrixRotationZ( &matRotZ, mfOriginalViewAngleHoriz );
		D3DXVec3TransformCoord( &xVect, &xVect, &matRotZ );
		mxCamFocus.x = mxBaseCamFocus.x - (fScreenXDelta*xVect.x*0.01f);
		mxCamFocus.y = mxBaseCamFocus.y - (fScreenXDelta*xVect.y*0.01f);
		break;
	case CONTROL_POLYSLICE:
		PlaneCutterRightMouseDownMove( fScreenXDelta, fScreenYDelta );
		break;
	case CONTROL_CAMERA:
	case CONTROL_PICKER:
	case CONTROL_VERTEXPICKER:
	case CONTROL_ROT:
	case CONTROL_VERTEXMOVE:
	case CONTROL_VERTEXROTATE:
	case CONTROL_VERTEXSCALE:
		mfViewAngleVert = mfOriginalViewAngleVert + (fScreenYDelta*0.01f);
		if ( mfViewAngleVert > TwoPi )
		{ 
			mfViewAngleVert -= TwoPi;
		}
		else if ( mfViewAngleVert < 0.0f )
		{
			mfViewAngleVert += TwoPi;
		}
		mfViewAngleHoriz = mfOriginalViewAngleHoriz + (fScreenXDelta*0.01f);
		if ( mfViewAngleHoriz > TwoPi )
		{ 
			mfViewAngleHoriz -= TwoPi;
		}
		else if ( mfViewAngleHoriz < 0.0f )
		{
			mfViewAngleHoriz += TwoPi;
		}
		break;
	case CONTROL_ZOOM:
		mfCamDist = mfOriginalCamDist + ((fScreenXDelta + fScreenYDelta) * 0.02f);
		if ( mfCamDist < 0.01f )
		{
			mfCamDist = 0.01f;
		}
		break;
	}
	ModelConverterSetupCamera();
	ModelConverterDisplayFrame( FALSE );
}

void ModelConvLeftMouseDownMove( float fScreenXDelta, float fScreenYDelta )
{
D3DXVECTOR3		xVect;
D3DXMATRIX	matRotX;
D3DXMATRIX	matRotZ;
float	fMoveSpeed = 0.03f;

	switch( mnPickerControlMode )
	{
	case CONTROL_CAMERA:
		mxCamFocus.z = mxBaseCamFocus.z + (fScreenYDelta*fMoveSpeed);		
		xVect.x = 1.0f;
		xVect.y = 0.0f;
		xVect.z = 0.0f;
		D3DXMatrixRotationZ( &matRotZ, mfOriginalViewAngleHoriz );
		D3DXVec3TransformCoord( &xVect, &xVect, &matRotZ );
		mxCamFocus.x = mxBaseCamFocus.x - (fScreenXDelta*xVect.x*fMoveSpeed);
		mxCamFocus.y = mxBaseCamFocus.y - (fScreenXDelta*xVect.y*fMoveSpeed);
		break;
	case CONTROL_PICKER:
	case CONTROL_VERTEXPICKER:
		mbModelConvShowSelectionRect = TRUE;
		break;
	case CONTROL_VERTEXSCALE:
		VertexManipulateScaleMouseUpdate( &m_MainSceneObject, fScreenXDelta, fScreenYDelta );
		break;
	case CONTROL_VERTEXMOVE:
	case CONTROL_VERTEXROTATE:
		VertexManipulateMouseMoveUpdate( &m_MainSceneObject, fScreenXDelta, fScreenYDelta );
		break;
	case CONTROL_POLYSLICE:
		// todo - move plane
		PlaneCutterLeftMouseDownMove( &m_MainSceneObject, fScreenXDelta, fScreenYDelta );
		break;
	}
}

VECT		ModelConvGetScreenMoveVector( float fScreenXDelta, float fScreenYDelta, float fScale )
{
VECT		xMove;
VECT		xCamForward = *EngineCameraGetDirection();
VECT		xCamUp = *EngineCameraGetUpVect();
VECT		xRight;
VECT		xUpMove;

	VectCross( &xRight, &xCamUp, &xCamForward );
	VectNormalize( &xRight );

	// todo - apply axis locks
	if ( mbModelConvMoveLockX == TRUE )
	{
		xRight.x = 0.0f;
		xCamUp.x = 0.0f;
	}
	if ( mbModelConvMoveLockY == TRUE )
	{
		xRight.y = 0.0f;
		xCamUp.y = 0.0f;
	}
	if ( mbModelConvMoveLockZ == TRUE )
	{
		xRight.z = 0.0f;
		xCamUp.z = 0.0f;
	}
	VectNormalize( &xCamUp );
	VectNormalize( &xRight );

	VectScale( &xMove, &xRight, fScreenXDelta * fScale );
	VectScale( &xUpMove, &xCamUp, fScreenYDelta * -fScale );

	VectAdd( &xMove, &xUpMove, &xMove );

		
	return( xMove );
}

void	ModelConvLeftMouseUp( int mouseX, int mouseY )
{
	switch( mnPickerControlMode )
	{
	case CONTROL_PICKER:
		if ( mbModelConvShowSelectionRect )
		{
		int		mouseStartX, mouseStartY;
		int		diffX, diffY;
		int		swap;
			
			mouseStartX = (int)mxMouseDownPos.x;
			mouseStartY = (int)mxMouseDownPos.y;
			
			diffX = abs( mouseX - mouseStartX );
			diffY = abs( mouseY - mouseStartY );

			// Select in region
			if ( ( diffX > 2 ) ||
				 ( diffY > 2 ) )
			{
				if ( mouseX < mouseStartX )
				{
					swap = mouseX;
					mouseX = mouseStartX;
					mouseStartX = swap;
				}
				if ( mouseY < mouseStartY )
				{
					swap = mouseY;
					mouseY = mouseStartY;
					mouseStartY = swap;
				}
				m_MainSceneObject.SelectAllFacesInScreenRegion( mouseStartX, mouseStartY, mouseX-mouseStartX, mouseY-mouseStartY );
			}
			else
			{
				m_MainSceneObject.SelectFaceAtScreenPoint( mouseX, mouseY );
			}
		}
		else
		{
			m_MainSceneObject.SelectFaceAtScreenPoint( mouseX, mouseY );
		}
		break;	
	case CONTROL_POLYSLICE:

		break;
	case CONTROL_VERTEXMOVE:
	case CONTROL_VERTEXROTATE:
	case CONTROL_VERTEXSCALE:
		VertexManipulateMouseUp( &m_MainSceneObject );
		break;
	case CONTROL_VERTEXPICKER:
		{
		int		mouseStartX, mouseStartY;
		int		swap;

			mouseStartX = (int)mxMouseDownPos.x;
			mouseStartY = (int)mxMouseDownPos.y;
			if ( ( mouseStartX == mouseX ) &&
				 ( mouseStartY == mouseY ) )
			{
				m_MainSceneObject.SelectVertexAtScreenPoint( mouseX, mouseY );
			}
			else
			{
				if ( mouseStartX > mouseX )
				{
					swap = mouseX;
					mouseX = mouseStartX;
					mouseStartX = swap;
				}
				if ( mouseStartY > mouseY )
				{
					swap = mouseY;
					mouseY = mouseStartY;
					mouseStartY = swap;
				}
				m_MainSceneObject.SelectAllVerticesInScreenRegion( mouseStartX, mouseStartY, mouseX-mouseStartX, mouseY-mouseStartY );
			}
		}
		break;
	}

	mbModelConvShowSelectionRect = FALSE;
}


void	ModelConvFocusCameraOnObject( int nHandle )
{
MODEL_RENDER_DATA*		pxModelData;
float		fCamDist = 1.0f;

	pxModelData = &maxModelRenderData[ nHandle ];

	mxCamFocus = pxModelData->xStats.xBoundBoxCentre;

	fCamDist = pxModelData->xStats.fBoundSphereRadius * 1.5f;
	mfCamDist = fCamDist;

	mfViewAngleVert = A30;
	mfViewAngleHoriz = 0.0f;
	ModelConverterSetupCamera();
}

/***************************************************************************
 * Function    : ModelConverterOpenDialog
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void		ModelConvLoadStart( eSUBMODEL_LIST nSubModelNum, const char* acFileName )
{
int		nHandle;

	if ( nSubModelNum == kMAIN )
	{
		m_MainSceneObject.Reset();
	}

	nHandle = ModelConvLoadModel( acFileName, 0, 1.0f );

	if ( nHandle != NOTFOUND )
	{
	int		nFileSize = 0;
	FILE*	pFile; 
	MODEL_STATS*	pxModelStats;

		pFile = fopen( acFileName, "rb" );
		if ( pFile )
		{
			nFileSize = SysGetFileSize( pFile );
			fclose( pFile );
		}
		if ( nFileSize == ModelGetStats(nHandle)->nFileSize )
		{
			ModelConvTextAdd( "Loaded model file %s (%dk)", acFileName,nFileSize/1024 );
		}
		else
		{
			ModelConvTextAdd( "Loaded model file %s (%dk) (Uncompressed to %dk)", acFileName,nFileSize/1024, ModelGetStats(nHandle)->nFileSize/1024 );
		}

		pxModelStats = ModelGetStats( nHandle );
		ModelConvTextAdd( "   %d verts, %d faces, %d materials, %d embedded textures (+ %d sub-models : %d LOD levels, %d collision models)", pxModelStats->nNumVertices, pxModelStats->nNumIndices / 3, pxModelStats->nNumMaterials, pxModelStats->nNumEmbeddedTextures, pxModelStats->nNumSubmodels, pxModelStats->bNumLODs, pxModelStats->nNumCollisionMaps );
		
		if ( nSubModelNum == kMAIN )
		{
			ModelConvFocusCameraOnObject( nHandle );
		}

		ModelConverterDisplayFrame(FALSE);

		// If we loaded an atm, store the filename and activate the save option
		if ( strstr( acFileName, ".atm" ) != NULL )
		{
			sprintf( maszCurrentModelFilename, acFileName );
			ModifyMenu( GetMenu( mhwndMainDialog ), IDM_FILE_SAVE, MF_ENABLED, IDM_FILE_SAVE, "Save" );				
		}
		else
		{
			ModifyMenu( GetMenu( mhwndMainDialog ), IDM_FILE_SAVE, MF_DISABLED, IDM_FILE_SAVE, "Save" );		
			maszCurrentModelFilename[0] = 0;
		}
	}
	else
	{	
		ModifyMenu( GetMenu( mhwndMainDialog ), IDM_FILE_SAVE, MF_DISABLED, IDM_FILE_SAVE, "Save" );		
		maszCurrentModelFilename[0] = 0;
	}

	switch( nSubModelNum )
	{
	case kMAIN:
		m_MainSceneObject.SetModel( nHandle );
		break;
	case kHORIZONTAL_TURRET:
		mnHorizTurretModelHandle = nHandle;
		if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
		{
		MODEL_RENDER_DATA*		pxModelData;

			pxModelData = &maxModelRenderData[ m_MainSceneObject.GetModelHandle() ];
			if ( pxModelData->xHorizTurretData.nModelHandle == NOTFOUND )
			{
//				pxModelData->xHorizTurretData.nAttachVertex = 0;
				pxModelData->xHorizTurretData.xAttachOffset.x = 0.0f;
				pxModelData->xHorizTurretData.xAttachOffset.y = 0.0f;
				pxModelData->xHorizTurretData.xAttachOffset.z = 0.0f;
			}
			pxModelData->xHorizTurretData.nModelHandle = mnHorizTurretModelHandle;
		}
		break;
	case kVERTICAL_TURRET:
		mnVertTurretModelHandle = nHandle;
		if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
		{
		MODEL_RENDER_DATA*		pxModelData;

			pxModelData = &maxModelRenderData[ m_MainSceneObject.GetModelHandle() ];
			if ( pxModelData->xVertTurretData.nModelHandle == NOTFOUND )
			{
				pxModelData->xVertTurretData.nAttachVertex = 0;
				pxModelData->xVertTurretData.xAttachOffset.x = 0.0f;
				pxModelData->xVertTurretData.xAttachOffset.y = 0.0f;
				pxModelData->xVertTurretData.xAttachOffset.z = 0.0f;
			}
			pxModelData->xVertTurretData.nModelHandle = mnVertTurretModelHandle;
		}
		break;
	case kWHEEL1:
		mnWheel1ModelHandle = nHandle;
		if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
		{
		MODEL_RENDER_DATA*		pxModelData;

			pxModelData = &maxModelRenderData[ m_MainSceneObject.GetModelHandle() ];
			if ( pxModelData->xWheel1AttachData.nModelHandle == NOTFOUND )
			{
				pxModelData->xWheel1AttachData.nAttachVertex = 0;
				pxModelData->xWheel1AttachData.xAttachOffset.x = 0.0f;
				pxModelData->xWheel1AttachData.xAttachOffset.y = 0.0f;
				pxModelData->xWheel1AttachData.xAttachOffset.z = 0.0f;
				pxModelData->xWheel2AttachData.nAttachVertex = 0;
				pxModelData->xWheel2AttachData.xAttachOffset.x = 0.0f;
				pxModelData->xWheel2AttachData.xAttachOffset.y = 0.0f;
				pxModelData->xWheel2AttachData.xAttachOffset.z = 0.0f;
			}
			pxModelData->xWheel1AttachData.nModelHandle = mnWheel1ModelHandle;
			pxModelData->xWheel2AttachData.nModelHandle = mnWheel1ModelHandle;
		}
		break;
	case kWHEEL2:
		mnWheel2ModelHandle = nHandle;
		if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
		{
		MODEL_RENDER_DATA*		pxModelData;

			pxModelData = &maxModelRenderData[ m_MainSceneObject.GetModelHandle() ];
			if ( pxModelData->xWheel3AttachData.nModelHandle == NOTFOUND )
			{
				pxModelData->xWheel3AttachData.nAttachVertex = 0;
				pxModelData->xWheel3AttachData.xAttachOffset.x = 0.0f;
				pxModelData->xWheel3AttachData.xAttachOffset.y = 0.0f;
				pxModelData->xWheel3AttachData.xAttachOffset.z = 0.0f;
				pxModelData->xWheel4AttachData.nAttachVertex = 0;
				pxModelData->xWheel4AttachData.xAttachOffset.x = 0.0f;
				pxModelData->xWheel4AttachData.xAttachOffset.y = 0.0f;
				pxModelData->xWheel4AttachData.xAttachOffset.z = 0.0f;
			}
			pxModelData->xWheel3AttachData.nModelHandle = mnWheel2ModelHandle;
			pxModelData->xWheel4AttachData.nModelHandle = mnWheel2ModelHandle;
		}
		break;
	case kMEDLOD:
		mnMedLODModelHandle = nHandle;
		if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
		{
		MODEL_RENDER_DATA*		pxModelData;

			pxModelData = &maxModelRenderData[ m_MainSceneObject.GetModelHandle() ];
			if ( pxModelData->xMedLODAttachData.nModelHandle == NOTFOUND )
			{
				pxModelData->xMedLODAttachData.nAttachVertex = 0;
				pxModelData->xMedLODAttachData.xAttachOffset.x = 0.0f;
				pxModelData->xMedLODAttachData.xAttachOffset.y = 0.0f;
				pxModelData->xMedLODAttachData.xAttachOffset.z = 0.0f;
			}
			pxModelData->xMedLODAttachData.nModelHandle = mnMedLODModelHandle;

			if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
			{
				pxModelData->xStats.bNumLODs = 2;		
			}
			else
			{
				pxModelData->xStats.bNumLODs = 1;
			}
		}
		break;
	case kLOWLOD:
		mnLowLODModelHandle = nHandle;
		if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
		{
		MODEL_RENDER_DATA*		pxModelData;

			pxModelData = &maxModelRenderData[ m_MainSceneObject.GetModelHandle() ];
			if ( pxModelData->xLowLODAttachData.nModelHandle == NOTFOUND )
			{
				pxModelData->xLowLODAttachData.nAttachVertex = 0;
				pxModelData->xLowLODAttachData.xAttachOffset.x = 0.0f;
				pxModelData->xLowLODAttachData.xAttachOffset.y = 0.0f;
				pxModelData->xLowLODAttachData.xAttachOffset.z = 0.0f;
			}
			pxModelData->xLowLODAttachData.nModelHandle = mnLowLODModelHandle;
			if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
			{
				pxModelData->xStats.bNumLODs = 2;		
			}
			else
			{
				pxModelData->xStats.bNumLODs = 1;
			}
		}
		break;
	case kCOLLISION:
		mnCollisionModelHandle = nHandle;
		if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
		{
		MODEL_RENDER_DATA*		pxModelData;

			pxModelData = &maxModelRenderData[ m_MainSceneObject.GetModelHandle() ];
			if ( pxModelData->xCollisionAttachData.nModelHandle == NOTFOUND )
			{
				pxModelData->xCollisionAttachData.nAttachVertex = 0;
				pxModelData->xCollisionAttachData.xAttachOffset.x = 0.0f;
				pxModelData->xCollisionAttachData.xAttachOffset.y = 0.0f;
				pxModelData->xCollisionAttachData.xAttachOffset.z = 0.0f;
			}
			pxModelData->xCollisionAttachData.nModelHandle = mnCollisionModelHandle;
		}
		break;
	}

	if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
	{
	MODEL_RENDER_DATA*		pxModelData;

		pxModelData = &maxModelRenderData[ m_MainSceneObject.GetModelHandle() ];
		if ( pxModelData->pMaterialData )
		{
			ModifyMenu( GetMenu( mhwndMainDialog ), ID_MATERIALS_MATERIALBROWSER, MF_ENABLED, ID_MATERIALS_MATERIALBROWSER, "Material Browser" );				
			ModifyMenu( GetMenu( mhwndMainDialog ), ID_MATERIALS_ADDMATERIALS, MF_GRAYED | MF_DISABLED, ID_MATERIALS_ADDMATERIALS, "Create Materials" );		
		}
		else
		{
			ModifyMenu( GetMenu( mhwndMainDialog ), ID_MATERIALS_MATERIALBROWSER, MF_GRAYED | MF_DISABLED, ID_MATERIALS_MATERIALBROWSER, "Material Browser" );				
			ModifyMenu( GetMenu( mhwndMainDialog ), ID_MATERIALS_ADDMATERIALS, MF_ENABLED, ID_MATERIALS_ADDMATERIALS, "Create Materials" );		
		}
	}
}


void		ModelConvSetLastUsedLoadPath( const char* szFile )
{
	GetPathFromFilename( szFile, macModelConvLastLoadFolder );
	ModelConvSaveUsedDirectories();
}


/***************************************************************************
 * Function    : ModelConverterOpenDialog
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void		ModelConvOpenDialog( eSUBMODEL_LIST nSubModelNum )
{
OPENFILENAME	ofnOpen;
char		acFileName[256];
char		acString[256];

	ZeroMemory( &ofnOpen, sizeof( ofnOpen ) );
	acFileName[0] = 0;
	GetCurrentDirectory( 256, acString );

	ofnOpen.lStructSize       = sizeof(OPENFILENAME);
    ofnOpen.hwndOwner         = mhwndMainDialog;
    ofnOpen.hInstance         = 0;
    ofnOpen.lpstrFilter       = (LPSTR)"All supported formats\0*.x;*.3ds;*.atm;*.fbx;*.obj;*.bsp\0DirectX Model Files (*.x)\0*.x\03DS Files (*.3ds)\0*.3ds\0The Universal Model Format (*.atm)\0*.atm\0Autodesk FBX (*.fbx)\0*.fbx\0Wavefront OBJ Files (*.obj)\0*.obj\0Quake 2/Halflife BSP File (*.bsp)\0*.bsp\0All Files (*.*)\0*.*\0";
    ofnOpen.lpstrCustomFilter = NULL;
    ofnOpen.nMaxCustFilter    = 0;
    ofnOpen.nFilterIndex      = 1;
    ofnOpen.lpstrFile         = (LPSTR)acFileName;
    ofnOpen.nMaxFile          = sizeof(acFileName);
    ofnOpen.lpstrInitialDir   = macModelConvLastLoadFolder;
    ofnOpen.lpstrTitle        = "Open model file(s)..";
    ofnOpen.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofnOpen.nFileOffset       = 0;
    ofnOpen.nFileExtension    = 0;

	if ( GetOpenFileName( &ofnOpen ) > 0 )
	{
		GetPathFromFilename( acFileName, macModelConvLastLoadFolder );
		ModelConvSaveUsedDirectories();

		ModelConvLoadStart( nSubModelNum, acFileName );
	}

}

void	ModelConvGetLastLoadFolder( char* szLoadFolder )
{
	strcpy( szLoadFolder, macModelConvLastLoadFolder );
}

void	ModelConvToggleLock( int nFlag )
{
	switch( nFlag )
	{
	case 0:
		mbModelConvMoveLockX = !mbModelConvMoveLockX;
		if ( mbModelConvMoveLockX )
		{
			SendDlgItemMessage( mhwndMainDialog, IDC_LOCKX, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMainDialog, IDC_LOCKX, BM_SETCHECK, BST_UNCHECKED, 0 );
		}
		break;
	case 1:
		mbModelConvMoveLockY = !mbModelConvMoveLockY;
		if ( mbModelConvMoveLockY )
		{
			SendDlgItemMessage( mhwndMainDialog, IDC_LOCKY, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMainDialog, IDC_LOCKY, BM_SETCHECK, BST_UNCHECKED, 0 );
		}
		break;
	case 2:
		mbModelConvMoveLockZ = !mbModelConvMoveLockZ;
		if ( mbModelConvMoveLockZ )
		{
			SendDlgItemMessage( mhwndMainDialog, IDC_LOCKZ, BM_SETCHECK, BST_CHECKED, 0 );
		}
		else
		{
			SendDlgItemMessage( mhwndMainDialog, IDC_LOCKZ, BM_SETCHECK, BST_UNCHECKED, 0 );
		}
		break;
	}
}


void	ModelConvDeleteFaces( void )
{
	m_MainSceneObject.DeleteSelectedFaces();
}


POINTS		GetInterfaceCoord( LPARAM lParam )
{
POINTS points;
float		fWindowScaleX = (float)( InterfaceGetWidth() ) / (float)( InterfaceGetWindowWidth() );
float		fWindowScaleY = (float)( InterfaceGetHeight() ) / (float)( InterfaceGetWindowHeight() );

	points = MAKEPOINTS(lParam);

	points.x = (short)( points.x * fWindowScaleX );
	points.y = (short)( points.y * fWindowScaleY );
	return( points );
}


void ModelConvReloadTexture( void )
{
//int		nRet;
int		nFileSize;
FILE*	pFile;
//D3DSURFACE_DESC		xSurfaceDesc;
//char	acString[512];
TEXTURE_HANDLE	hTexture;

	SysSetCurrentDir( macModelConvRootFolder );
	EngineReloadShaders();

	m_MainSceneObject.ReleaseTexture();
	
	pFile = fopen( macLoadedTextureFilename, "rb" );
	if ( pFile != NULL )
	{
		fseek( pFile, 0, SEEK_END );
		nFileSize = ftell(pFile);
		fclose( pFile );
	
		hTexture = EngineLoadTexture( macLoadedTextureFilename, 0, NULL );
		m_MainSceneObject.SetTexture( hTexture );

/* TODO - replace this funtionality since we've moved to EngineLoadTexture
		if( FAILED( nRet ) )
		{
			InterfaceTextureLoadError( nRet,macLoadedTextureFilename );
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_REFRESH_TEXTURE ), FALSE );
		}
		else
		{
			EnableWindow( GetDlgItem( mhwndMainDialog, IDC_REFRESH_TEXTURE ), TRUE );
			mpxRenderTexture->GetLevelDesc( 0, &xSurfaceDesc );
			ModelConvTextAdd( "Texture file %s reloaded. (%dx%d - %dk)", macLoadedTextureFilename, xSurfaceDesc.Width,xSurfaceDesc.Height, (nFileSize/1024) );
		}		
*/
		ModelConverterDisplayFrame( FALSE );
	}
}

BOOL		ModelConverterFilenameIsASupportedModelFormat( const char* szFilename )
{
	if ( szFilename )
	{
	const char*		szExtension = SysGetFileExtension( szFilename );

		if ( ( stricmp( szExtension, "atm") == 0 ) ||
			 ( stricmp( szExtension, "fbx") == 0 ) ||
			 ( stricmp( szExtension, "3ds") == 0 ) ||
			 ( stricmp( szExtension, "x") == 0 ) )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}



BOOL		ModelConverterFilenameIsASupportedTextureFormat( const char* szFilename )
{
	if ( szFilename )
	{
	const char*		szExtension = SysGetFileExtension( szFilename );

		if ( ( stricmp( szExtension, "png") == 0 ) ||
			 ( stricmp( szExtension, "bmp") == 0 ) ||
			 ( stricmp( szExtension, "jpg") == 0 ) ||
			 ( stricmp( szExtension, "tga") == 0 ) )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}

void		ModelConvLoadMainRenderTexture( const char* szFilename )
{
TEXTURE_HANDLE	hTexture;

	m_MainSceneObject.ReleaseTexture();
			
	hTexture = EngineLoadTexture( szFilename, 0, NULL );
	m_MainSceneObject.SetTexture( hTexture );

	strcpy( macLoadedTextureFilename, szFilename );
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_REFRESH_TEXTURE ), TRUE );

		// TODO - Needs updating to work from EngineLoadTexture
//			mpxRenderTexture->GetLevelDesc( 0, &xSurfaceDesc );
//			sprintf( acString, "Texture file %s loaded. (%dx%d - %dk)", acFileName, xSurfaceDesc.Width,xSurfaceDesc.Height, (nFileSize/1024) );
//			ModelConvTextAdd( acString );

	ModelConvTextAdd( "Texture file %s loaded.", szFilename );

}


void		ModelConverterDropFile( const char* szFilename )
{
	if ( ModelConverterFilenameIsASupportedModelFormat( szFilename ) == TRUE )
	{
		ModelConvLoadStart( kMAIN, szFilename );		
	}
	else if ( ModelConverterFilenameIsASupportedTextureFormat( szFilename ) == TRUE )
	{
		ModelConvLoadMainRenderTexture( szFilename );
	}

}


void		ModelConverterDropMultipleFiles( HDROP dropHandle, int nNumFiles )
{
int			nLoop;
char	acFilename[512];
int			nHandle;

	if ( ModelConvGetCurrentModel() == NOTFOUND )
	{
		DragQueryFile( dropHandle, 0, acFilename, 512 );
		ModelConvLoadStart( kMAIN, acFilename );		
	}

	for( nLoop = 0; nLoop < nNumFiles; nLoop++ )
	{
		DragQueryFile( dropHandle, nLoop, acFilename, 512 );

		if ( ModelConverterFilenameIsASupportedModelFormat( acFilename ) == TRUE )
		{	
			nHandle = ModelLoad( acFilename, 0, 1.0f );
			if ( nHandle != NOTFOUND )
			{
				AnimationBuilderAddNewKeyframe( nHandle );
			}
		}
	}
	AnimationBuilderApplyNewKeyframes();
}

/***************************************************************************
 * Function    : ModelConverterGraphicWindowDlgProc
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
LRESULT CALLBACK ModelConverterGraphicWindowDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
POINTS points;

	switch (message)
	{
	case WM_DROPFILES:
		{
		char	acFilename[512];
		int		nNumFilesDropped = DragQueryFile( (HDROP)wParam, 0xFFFFFFFF, NULL, 0 );

			if ( nNumFilesDropped == 1 )
			{
				DragQueryFile( (HDROP)wParam, 0, acFilename, 512 );
				ModelConverterDropFile( acFilename );
			}
			else
			{
				ModelConverterDropMultipleFiles( (HDROP)wParam, nNumFilesDropped );
			}
		}
		break;
	case WM_ACTIVATEAPP:
		if ( wParam == FALSE )
		{
			mbThisWindowIsFocused = FALSE;
		}
		else
		{
			mbThisWindowIsFocused = TRUE;
		}
		break;
	case WM_INITDIALOG:
		return TRUE;
	case WM_PAINT:
		ModelConverterDisplayFrame( FALSE );
		break;
    case WM_MOUSEMOVE:
		points = GetInterfaceCoord(lParam);

		mxMousePos.x = points.x;
		mxMousePos.y = points.y;

		if ( mboRightMouseDown == TRUE )
		{
		VECT	mxMouseDelta;
			mxMouseDelta.x = mxMousePos.x - mxMouseDownPos.x;
			mxMouseDelta.y = mxMousePos.y - mxMouseDownPos.y;

			ModelConvRightMouseDownMove( mxMouseDelta.x, mxMouseDelta.y );
		}
		else if ( mboLeftMouseDown == TRUE )
		{
		VECT	mxMouseDelta;
			mxMouseDelta.x = mxMousePos.x - mxMouseDownPos.x;
			mxMouseDelta.y = mxMousePos.y - mxMouseDownPos.y;

			ModelConvLeftMouseDownMove( mxMouseDelta.x, mxMouseDelta.y );
		}
		else if ( mboMidMouseDown == TRUE )
		{
		VECT	mxMouseDelta;
			mxMouseDelta.x = mxMousePos.x - mxMouseDownPos.x;
			mxMouseDelta.y = mxMousePos.y - mxMouseDownPos.y;

			ModelConvMidMouseDownMove( mxMouseDelta.x, mxMouseDelta.y );
		}

		break;
	case WM_RBUTTONUP:
		mboRightMouseDown = FALSE;
		break;
	case WM_MBUTTONUP:
		mboMidMouseDown = FALSE;
		break;
	case WM_MBUTTONDOWN:
		mboMidMouseDown = TRUE;
		points = GetInterfaceCoord(lParam);

		mxMouseDownPos.x = points.x;
		mxMouseDownPos.y = points.y;

		ModelConvMouseDownStore();
		break;
	case WM_RBUTTONDOWN:
		mboRightMouseDown = TRUE;
		points = GetInterfaceCoord(lParam);

		mxMouseDownPos.x = points.x;
		mxMouseDownPos.y = points.y;

		ModelConvMouseDownStore();
		break;
	case WM_KEYDOWN:
		switch ( (short)( wParam ) )
		{
		case VK_F5:
			ModelConvReloadTexture();
			break;
		case VK_DELETE:
			ModelConvDeleteFaces();
			break;
		case VK_LEFT:
			if ( mnViewPage == 1 )
			{
				SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_SETCURSEL, 0, 0 );
				mnViewPage = 0;
			}
			else
			{
				SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_SETCURSEL, 1, 0 );
				mnViewPage = 1;
			}
			ModelConverterDisplayFrame( FALSE );
			break;
		case VK_RIGHT:
			if ( mnViewPage == 3 )
			{
				SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_SETCURSEL, 0, 0 );
				mnViewPage = 0;
			}
			else
			{
				SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_SETCURSEL, 3, 0 );
				mnViewPage = 3;
			}
			ModelConverterDisplayFrame( FALSE );
			break;
		case VK_UP:
			if ( mnViewPage == 2 )
			{
				SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_SETCURSEL, 5, 0 );
				mnViewPage = 5;
			}
			else
			{
				SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_SETCURSEL, 2, 0 );
				mnViewPage = 2;
			}
			ModelConverterDisplayFrame( FALSE );
			break;
		case VK_DOWN:
			if ( mnViewPage == 4 )
			{
				SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_SETCURSEL, 6, 0 );
				mnViewPage = 6;
			}
			else
			{
				SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_SETCURSEL, 4, 0 );
				mnViewPage = 4;
			}
			ModelConverterDisplayFrame( FALSE );
			break;
		}
		break;
	case WM_CHAR:
		switch ( (short)( wParam ) )
		{
		case 'X':
		case 'x':
			ModelConvToggleLock( 0 );
			break;
		case 'Y':
		case 'y':
			ModelConvToggleLock( 1 );
			break;
		case 'Z':
		case 'z':
			ModelConvToggleLock( 2 );
			break;
		case 'M':
		case 'm':
			mnPickerControlMode = CONTROL_CAMERA;
			break;
		case 'P':
		case 'p':
			mnPickerControlMode = CONTROL_PICKER;
			break;
		case 'V':
		case 'v':
			ModelConvResetCamera();
			ModelConverterSetupCamera();
			ModelConverterDisplayFrame( FALSE );
			break;
		case 'T':
		case 't':
			m_MainSceneObject.ToggleWireframe();
			break;
		}
		break;
	case 0x020A:		// WM_MOUSEWHEEL - for some reason i cant get it to include..
		{
		int		nDelta;
		short	wShortParam = HIWORD(wParam);
		float	fOffset;

			nDelta = (int)( wShortParam );
			fOffset = (float)( nDelta * 4 );
			// todo - step should shrink as we get closer to model
			fOffset *= 0.02f / mfScaleUnit;
			if ( SysCheckKeyState( KEY_SHIFT ) == TRUE )
			{
				fOffset *= 10.0f;
			}

			ModelConvSetZoomScreenDelta( fOffset * -1.0f, 1.5f );
		}
		break;
	case WM_LBUTTONDOWN:
		mboLeftMouseDown = TRUE;
		points = GetInterfaceCoord(lParam);
 
		mxMouseDownPos.x = points.x;
		mxMouseDownPos.y = points.y;

		ModelConvMouseDownStore();

		switch( mnPickerControlMode )
		{
		case CONTROL_VERTEXMOVE:
		case CONTROL_VERTEXROTATE:
		case CONTROL_VERTEXSCALE:
			VertexManipulateMouseDownStore( &m_MainSceneObject );
			break;
		}
		break;
	case WM_LBUTTONUP:
		mboLeftMouseDown = FALSE;
		points = GetInterfaceCoord(lParam);
		ModelConvLeftMouseUp( points.x, points.y );
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
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

    return DefWindowProc( hDlg, message, wParam, lParam );
}

void ExportSettingsUpdateRadios( void )
{
char	acString[256];

	if ( ModelGetStats(m_MainSceneObject.GetModelHandle())->ulLockID != 0 )
	{
		SendDlgItemMessage( mhwndExportSettingsDialog, IDC_LOCAL_LOCK, BM_SETCHECK, BST_CHECKED, 0 );
		SendDlgItemMessage( mhwndExportSettingsDialog, IDC_NO_LOCK, BM_SETCHECK, BST_UNCHECKED, 0 );
		sprintf( acString, "%08x", ModelGetStats(m_MainSceneObject.GetModelHandle())->ulLockID );
		SetDlgItemText( mhwndExportSettingsDialog, IDC_LOCK_ID_STRING, acString );
	}
	else
	{
		SendDlgItemMessage( mhwndExportSettingsDialog, IDC_NO_LOCK, BM_SETCHECK, BST_CHECKED, 0 );
		SendDlgItemMessage( mhwndExportSettingsDialog, IDC_LOCAL_LOCK, BM_SETCHECK, BST_UNCHECKED, 0 );
	}
	
	SetDlgItemText( mhwndExportSettingsDialog, IDC_EDIT1, ModelGetStats(m_MainSceneObject.GetModelHandle())->acCreatorInfo );

}



/***************************************************************************
 * Function    : ModelConverterAssignMaterialDlgProc
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
LRESULT CALLBACK ModelConverterAssignMaterialDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
//char	acString[512];

	switch (message)
	{
	case WM_INITDIALOG:
		// TODO - fill in material list
		{
		int	nModelHandle = ModelConvGetCurrentModel();
		MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[ nModelHandle ];
		int		loop;
		char	acString[256];

			for ( loop = 0; loop < pxModelData->xStats.nNumMaterials; loop++ )
			{	
				sprintf( acString, "Material %d", loop+1 );
				SendDlgItemMessage( hDlg, IDC_COMBO1, CB_ADDSTRING, loop, (LPARAM)acString );
			}
			SendDlgItemMessage( hDlg, IDC_COMBO1, CB_SETCURSEL, 0, 0 );
		}
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case BN_CLICKED:
			{
			int		nVal;
			BOOL	boSelected;
			int		nSelected;

				nSelected = LOWORD( wParam );
				nVal = SendDlgItemMessage( hDlg, nSelected, BM_GETCHECK, 0, 0 );

				if ( nVal == BST_CHECKED )
				{
					boSelected = TRUE;
				}
				else
				{
					boSelected = FALSE;
				}

				switch( nSelected )
				{
				case IDAPPLY:
					nVal = SendDlgItemMessage( hDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0 );
					m_MainSceneObject.ApplyMaterialToSelectedFaces( nVal );
					m_MainSceneObject.EnableSelectedFaceHighlight( false );
					break;
				case IDOK:
					nVal = SendDlgItemMessage( hDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0 );
					m_MainSceneObject.ApplyMaterialToSelectedFaces( nVal );
					m_MainSceneObject.EnableSelectedFaceHighlight( true );
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case IDCANCEL:
					m_MainSceneObject.EnableSelectedFaceHighlight( true );
					EndDialog(hDlg, LOWORD(wParam));
					break;
				}
				break;
			}
		}
		break;
	case WM_CLOSE:
		m_MainSceneObject.EnableSelectedFaceHighlight( true );
		EndDialog(hDlg, LOWORD(wParam));
		return( 0 );
	case WM_DESTROY:
		return(0);
	default:
		break;
	}
	return( FALSE );
}


/***************************************************************************
 * Function    : ExportSettingsMainDlgProc
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
LRESULT CALLBACK ExportSettingsMainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
char	acString[512];

	switch (message)
	{
	case WM_INITDIALOG:
		mhwndExportSettingsDialog = hDlg;
		ExportSettingsUpdateRadios();
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case EN_CHANGE:
			{
//			char	acString[256];
//			float	fVal;

//				GetDlgItemText( mhwndMainDialog, IDC_CAM_DIST, acString, 256);
//				fVal = strtod( acString, NULL );
//				mfCamDist = fVal;
//				ModelConverterSetupCamera();
//				ModelConverterDisplayFrame();
			}
			break;
		case BN_CLICKED:
			{
			int		nVal;
			BOOL	boSelected;
			int		nSelected;

				nSelected = LOWORD( wParam );
				nVal = SendDlgItemMessage( hDlg, nSelected, BM_GETCHECK, 0, 0 );

				if ( nVal == BST_CHECKED )
				{
					boSelected = TRUE;
				}
				else
				{
					boSelected = FALSE;
				}

				switch( nSelected )
				{
				case IDOK:
					GetDlgItemText( mhwndExportSettingsDialog, IDC_EDIT1, acString, 256 );
					acString[79] = 0;
					strcpy( ModelGetStats(m_MainSceneObject.GetModelHandle())->acCreatorInfo, acString );
					EndDialog(hDlg, 0);
					break;
				case IDC_LOCAL_LOCK:
#ifdef SECURITY_FEATURES
					ModelGetStats(m_MainSceneObject.GetModelHandle())->ulLockID = GetUniqueID();
					ModelGetStats(m_MainSceneObject.GetModelHandle())->ulLockCode = 0x40A0;
#endif
					ExportSettingsUpdateRadios();
					break;
				case IDC_NO_LOCK:
					ModelGetStats(m_MainSceneObject.GetModelHandle())->ulLockID = 0;
					ModelGetStats(m_MainSceneObject.GetModelHandle())->ulLockCode = 0;
					ExportSettingsUpdateRadios();
					break;
				}
				break;
			}
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






void	ModelConvExtractLODs( void )
{
MODEL_RENDER_DATA*	pxModelData;
char	acFilename[256];
char	acBaseFilename[256];
int		nStringLen;
uint32	ulFlags = ModelConvGetExportFlags();
uint32	ulLockFlags = 0;

	if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
	{
		pxModelData = maxModelRenderData + m_MainSceneObject.GetModelHandle();

		nStringLen = strlen( maszCurrentModelFilename );
		if ( nStringLen > 5 )
		{
			strcpy( acBaseFilename,maszCurrentModelFilename );
			acBaseFilename[nStringLen-4] = 0;

			if ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
			{
				sprintf( acFilename, "%s_med-LOD.atm", acBaseFilename );
				ModelExportATM( pxModelData->xMedLODAttachData.nModelHandle, acFilename, ulFlags, ulLockFlags );
			}
			if ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND )
			{
				sprintf( acFilename, "%s_low-LOD.atm", acBaseFilename );
				ModelExportATM( pxModelData->xLowLODAttachData.nModelHandle, acFilename, ulFlags, ulLockFlags );
			}
		}
	}
}

/***************************************************************************
 * Function    : LODSettingsDlgProc
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
LRESULT CALLBACK LODSettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
MODEL_RENDER_DATA*	pxModelData;

	switch (message)
	{
	case WM_INITDIALOG:
		{
		char	acString[256];
		//float	fScale;

//			mhwndLODDialog = hDlg;
			if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
			{
				pxModelData = maxModelRenderData + m_MainSceneObject.GetModelHandle();
				if ( pxModelData->xLodData.fHighDist == 0.0f )
				{
					pxModelData->xLodData.fHighDist = 5.0f / mfScaleUnit;
				}
				if ( pxModelData->xLodData.fMedDist == 0.0f )
				{	
					pxModelData->xLodData.fMedDist = 20.0f / mfScaleUnit;
				}				
				sprintf( acString, "%.02f", (pxModelData->xLodData.fHighDist * mfScaleUnit) );
				SetDlgItemText( hDlg, IDC_EDIT1, acString );
				sprintf( acString, "%.02f", (pxModelData->xLodData.fMedDist * mfScaleUnit) );
				SetDlgItemText( hDlg, IDC_EDIT2, acString );
			}
//			SetDlgItemText( hDlg, IDC_EDIT4, acString );
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
				float	fDist1;
				float	fDist2;
				char	acString[256];

					if ( m_MainSceneObject.GetModelHandle() != NOTFOUND )
					{
						pxModelData = maxModelRenderData + m_MainSceneObject.GetModelHandle();
						GetDlgItemText( hDlg, IDC_EDIT1, acString, 256 );
						fDist1 = (float)( strtod( acString, NULL ) );
						GetDlgItemText( hDlg, IDC_EDIT2, acString, 256 );
						fDist2 = (float)( strtod( acString, NULL ) );
						pxModelData->xLodData.fHighDist = fDist1 / mfScaleUnit;
						pxModelData->xLodData.fMedDist = fDist2 / mfScaleUnit;
					}
				}
				EndDialog(hDlg, 0);
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

LRESULT CALLBACK RotateDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
int		nVal;

	switch (message)
	{
	case WM_INITDIALOG:
		{
		char	acString[256];
		//float	fScale;

			sprintf( acString, "0.0" );
			SetDlgItemText( hDlg, IDC_EDIT1, acString );
			SendDlgItemMessage( hDlg, IDC_RADIO1, BM_SETCHECK, BST_CHECKED, 0 );
			SendDlgItemMessage( hDlg, IDC_RADIO2, BM_SETCHECK, BST_UNCHECKED, 0 );
			SendDlgItemMessage( hDlg, IDC_RADIO3, BM_SETCHECK, BST_UNCHECKED, 0 );

			SendDlgItemMessage( hDlg, IDC_RADIO_ALL_LODS, BM_SETCHECK, BST_CHECKED, 0 );
			SendDlgItemMessage( hDlg, IDC_RADIO_APPLY_TO_HIGH_LOD, BM_SETCHECK, BST_UNCHECKED, 0 );				
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
				float	fXOffset;
				char	acString[256];
				BOOL	bAffectSubModels = TRUE;

					GetDlgItemText( hDlg, IDC_EDIT1, acString, 256 );
					fXOffset = (float)( strtod( acString, NULL ) );
					fXOffset = ( fXOffset * TwoPi ) / 360.0f;
					nVal = SendDlgItemMessage( hDlg, IDC_RADIO1, BM_GETCHECK, 0, 0 );

					bAffectSubModels = SendDlgItemMessage( hDlg, IDC_RADIO_ALL_LODS, BM_GETCHECK, 0, 0 );

					if ( nVal == 1 )
					{
						ModelConvRotateModelFree( m_MainSceneObject.GetModelHandle(), fXOffset, 0.0f,0.0f, bAffectSubModels );
					}
					else
					{
						nVal = SendDlgItemMessage( hDlg, IDC_RADIO2, BM_GETCHECK, 0, 0 );
						if ( nVal == 1 )
						{
							ModelConvRotateModelFree( m_MainSceneObject.GetModelHandle(), 0.0f,fXOffset,0.0f, bAffectSubModels );
						}
						else
						{
							ModelConvRotateModelFree( m_MainSceneObject.GetModelHandle(), 0.0f,0.0f,fXOffset, bAffectSubModels );
						}
					}
				}
				EndDialog(hDlg, 0);
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


/***************************************************************************
 * Function    : ModelConvBrowseForFolder
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void	ModelConvBrowseForFolder( char* pcOut )
{
BROWSEINFO	xBrowseInfo;
LPITEMIDLIST		pItemList;

	ZeroMemory( &xBrowseInfo, sizeof( BROWSEINFO ) );

	xBrowseInfo.hwndOwner = mhwndBatchConvertDialog;
	xBrowseInfo.lpszTitle = "Select Folder containing .x Files";
	xBrowseInfo.pszDisplayName = macBatchConvFolder;
	xBrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;

	pItemList = SHBrowseForFolder( &xBrowseInfo );

	if ( pItemList != NULL )
	{
		SHGetPathFromIDList( pItemList, pcOut );
	}
}

/***************************************************************************
 * Function    : PopupDlgProc
 * Params      :
 * Returns     :
 * Description : Message handler for the front end dialog box
 ***************************************************************************/
LRESULT CALLBACK PopupDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE; 
		break;
	case WM_DESTROY:
		return(0);
	default:
		break;
	}
	return( FALSE );
}

typedef struct
{
	uint32	ulExportFlags;
	int		nX;
	int		nY;
	int		nWidth;
	int		nHeight;
	
	int		nScaleUnitMode;
} MODEL_CONV_CONFIG;


MODEL_CONV_CONFIG	mxConfig;



void ModelConvSaveExportFlags( void )
{
FILE*	pFile;
RECT	rect;
char	acString[256];

	GetWindowRect(mhwndMainDialog, &rect);
	mxConfig.nX = rect.left;
	mxConfig.nY = rect.top;
	mxConfig.nWidth = rect.right - rect.left;
	mxConfig.nHeight = rect.bottom - rect.top;
	mxConfig.ulExportFlags = mulLastExportFlags;

	sprintf ( acString, "%s/export.opt", macModelConvRootFolder );

	pFile = fopen( acString, "wb" );
	if ( pFile != NULL )
	{
		fwrite( &mxConfig, sizeof(MODEL_CONV_CONFIG), 1, pFile );
		fclose( pFile );
	}
}

float	ModelConvGetScaleUnit( void )
{
	return( mfScaleUnit );
}

void		ModelConvSetShadowMode( int nMode )
{
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SHADOWS_NONE, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SHADOWS_SHADOW_MAP, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_SHADOWS_SHADOW_VOLUME, MF_UNCHECKED);
	
	switch( nMode )
	{
	case 0:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SHADOWS_NONE, MF_CHECKED);
		EngineSceneShadowMapActivate( FALSE );
		ShadowVolumeRenderingEnable( FALSE, 10.0f );
		break;
	case 1:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SHADOWS_SHADOW_MAP, MF_CHECKED);
		EngineSceneShadowMapActivate( TRUE );
		ShadowVolumeRenderingEnable( FALSE, 10.0f );
		break;
	case 2:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_SHADOWS_SHADOW_VOLUME, MF_CHECKED);
		EngineSceneShadowMapActivate( FALSE );

		ShadowVolumeRenderingEnable( TRUE, 10.0f );
		VECT	xVect = { 110.0f, 90.0f, 20.0f };
		ShadowVolumeSetLightPosition( &xVect );
		break;
	}
}


void	ModelConvSetScaleUnitsOption( int nMode, BOOL bSave )
{
BOOL	bDidChange = FALSE;

	if ( mnScaleUnitMode != nMode ) bDidChange = TRUE;

	mnScaleUnitMode = nMode;
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_MENUOPTIONS_SCALEMODE_101M, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_MENUOPTIONS_SCALEMODE_10100FTTHEUNIVERSALMODE, MF_UNCHECKED);
	switch ( nMode )
	{
	case 0:
	default:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_MENUOPTIONS_SCALEMODE_10100FTTHEUNIVERSALMODE, MF_CHECKED);
		// -- For TheUniversalstuff.. with its whacky tiny scale caused by immense tininess
		mfScaleUnit = 24.0f;		// i.e. 1.0f = 24m 
		break;
	case 1:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_MENUOPTIONS_SCALEMODE_101M, MF_CHECKED);
		mfScaleUnit = 1.0f;
		break;
	}

	if ( bDidChange )
	{
		ModelConvResetCamera();

		if ( bSave )
		{
			mxConfig.nScaleUnitMode = nMode;
			ModelConvSaveExportFlags();
		}
	}
}



void ModelConvLoadExportFlags( void )
{
FILE*	pFile;
char	acString[256];

	ZeroMemory( &mxConfig, sizeof( MODEL_CONV_CONFIG ) );
	sprintf ( acString, "%s/export.opt", macModelConvRootFolder );
	pFile = fopen( acString, "rb" );
	if ( pFile != NULL )
	{
		fread( &mxConfig, sizeof(MODEL_CONV_CONFIG), 1, pFile );
		fclose( pFile );
		mxConfig.ulExportFlags |= ATM_EXPORT_FLAGS_COMPRESSION;
		mulLastExportFlags = mxConfig.ulExportFlags;
		ModelConvSetScaleUnitsOption( mxConfig.nScaleUnitMode, FALSE );
	}
}

HWND	mhwndBatchRescaleDialog;
char		macBatchRescaleFolder[256] = "";
HWND	mhwndBatchRescalePopup;

void ModelConvBatchRescaleProcessFilesInFolder( char* szFolder )
{
WIN32_FIND_DATA FileData; 
char	acString[256];
char	acString2[256];
//char	acFolder[256];
HANDLE hSearch; 
BOOL fFinished = FALSE; 
int		nLoadedModelHandle;
int		nNumModelFiles = 0;
char*	pcString;
FILE*	pFile;
int		nSize;
uint32	ulExportFlags = ModelConvGetExportFlags();


	sprintf( acString, "%s*.*", szFolder );	 

	fFinished = FALSE;
	hSearch = FindFirstFile(acString, &FileData); 

	if (hSearch != INVALID_HANDLE_VALUE) 
	{ 
		while ( !fFinished )
		{ 
			if(!(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				pcString = &FileData.cFileName[ strlen( FileData.cFileName ) - 4 ];
				if ( stricmp( pcString, ".atm" ) == 0 )
				{
					sprintf( acString, "Converting.. %s", FileData.cFileName );
					SetDlgItemText( mhwndBatchRescalePopup, IDC_CONVERTING, acString );
					sprintf( acString2, "%s%s", szFolder, FileData.cFileName );
					nLoadedModelHandle = ModelConvLoadModel( acString2, 0, 0.0035f );

					if ( nLoadedModelHandle != NOTFOUND )
					{
						ModelConvTextAdd( "Loaded .atm file - %s. (%dk)", acString2, ModelGetStats(nLoadedModelHandle)->nFileSize/1024 );
						UpdateWindow( mhwndMainDialog );

						ModelConvScaleModelFree( nLoadedModelHandle, 2500.0f, 2500.0f, 2500.0f );

						ModelExportATM( nLoadedModelHandle, acString2, ulExportFlags, 0 );

						pFile = fopen( acString2, "rb" );
						if ( pFile != NULL )
						{
							// Get the size of the tex
							fseek( pFile, 0, SEEK_END );
							nSize = ftell(pFile);
							rewind( pFile );
							fclose( pFile );
		
							if ( nSize > 1024 )
							{
								ModelConvTextAdd( "Model file - %s exported. (%dk)", acString2, (nSize/1024) );
							}
							else
							{
								ModelConvTextAdd( "Model file - %s exported. (%d bytes)", acString2, nSize );
							}
							UpdateWindow( mhwndMainDialog );
						}
						ModelFree( nLoadedModelHandle );
					}
//					nProgressBar++;
//					SendDlgItemMessage( mhwndBatchRescalePopup, IDC_PROGRESS1, PBM_SETPOS, (WPARAM)nProgressBar, 0 );
					UpdateWindow( mhwndBatchRescalePopup );
				}
			}

		    if (!FindNextFile(hSearch, &FileData)) 
		    {
	            fFinished = TRUE; 
		    }
		} 
		// Close the search handle.  
		FindClose(hSearch);
	}
}

void ModelConvBatchRescaleRecurseFolders( char* szFolder )
{
WIN32_FIND_DATA FileData; 
HANDLE hSearch; 
BOOL fFinished = FALSE; 
char	acString[256];
char	acString2[256];

	ModelConvBatchRescaleProcessFilesInFolder( szFolder );
	sprintf( acString, "%s*", szFolder );	 
	 
	hSearch = FindFirstFile(acString, &FileData); 
	// Search for subfolders and recurse them too
	if (hSearch != INVALID_HANDLE_VALUE) 
	{ 
		while ( (!fFinished) )
		{ 
			if( ( !lstrcmp(FileData.cFileName, ".")  ) ||
				( !lstrcmp(FileData.cFileName, "..") ) )
			{
				// Jus skip these for the moment
			}
			else
			{
				if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					sprintf( acString2,"%s%s\\", szFolder, FileData.cFileName );
					ModelConvBatchRescaleRecurseFolders( acString2 );
				}
			}
		    if (!FindNextFile(hSearch, &FileData)) 
		    {
	            fFinished = TRUE; 
		    }
		}
		// Close the search handle.  
		FindClose(hSearch); 
	}
}



void ModelConvBatchRescaleProcessFolderNoRecurse( const char* szFolder )
{
WIN32_FIND_DATA FileData; 
char	acString[256];
char	acString2[256];
char	acFolder[256];
HANDLE hSearch; 
BOOL fFinished = FALSE; 
int		nLoadedModelHandle;
int		nNumModelFiles = 0;
int		nProgressBar;
HWND	hwndPopup;
char*	pcString;
FILE*	pFile;
int		nSize;
uint32	ulExportFlags = ModelConvGetExportFlags();

	hwndPopup = CreateDialog(ghInstance, (LPCTSTR)IDD_BATCH_PROGRESS, mhwndBatchRescaleDialog, (DLGPROC)PopupDlgProc );

	SendDlgItemMessage( hwndPopup, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM( 0, 10 ) );
	SendDlgItemMessage( hwndPopup, IDC_PROGRESS1, PBM_SETPOS, (WPARAM)0, 0 );
	nProgressBar = 0;
    ShowWindow( hwndPopup, SW_SHOW );
	UpdateWindow( hwndPopup );

	strcpy( acFolder, szFolder );
	pcString= &acFolder[ strlen( acFolder ) - 1 ];
	if ( *pcString != '\\' )
	{
		sprintf(acFolder , "%s\\*.*", acFolder );
	}
	hSearch = FindFirstFile(acFolder, &FileData); 

	if (hSearch != INVALID_HANDLE_VALUE) 
	{ 
		while ( !fFinished )
		{ 
			pcString = &FileData.cFileName[ strlen( FileData.cFileName ) - 4 ];
			if ( stricmp( pcString, ".atm" ) == 0 )
			{
				nNumModelFiles++;
			}
	 
		    if (!FindNextFile(hSearch, &FileData)) 
		    {
	            fFinished = TRUE; 
		    }
		} 
		// Close the search handle.  
		FindClose(hSearch);
	}
	SendDlgItemMessage( hwndPopup, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM( 0, nNumModelFiles ) );

	fFinished = FALSE;
	hSearch = FindFirstFile(acFolder, &FileData); 

	if (hSearch != INVALID_HANDLE_VALUE) 
	{ 
		while ( !fFinished )
		{ 
			pcString = &FileData.cFileName[ strlen( FileData.cFileName ) - 4 ];
			if ( stricmp( pcString, ".atm" ) == 0 )
			{
				sprintf( acString, "Converting.. %s", FileData.cFileName );
				SetDlgItemText( hwndPopup, IDC_CONVERTING, acString );
				sprintf( acString2, "%s\\%s", szFolder, FileData.cFileName );
				nLoadedModelHandle = ModelConvLoadModel( acString2, 0, 0.0035f );

				if ( nLoadedModelHandle != NOTFOUND )
				{
					ModelConvTextAdd( "Loaded .atm file - %s. (%dk)", acString2, ModelGetStats(nLoadedModelHandle)->nFileSize/1024 );
					UpdateWindow( mhwndMainDialog );

					ModelConvScaleModelFree( nLoadedModelHandle, 2500.0f, 2500.0f, 2500.0f );

					ModelExportATM( nLoadedModelHandle, acString2, ulExportFlags, 0 );

					pFile = fopen( acString2, "rb" );
					if ( pFile != NULL )
					{
						// Get the size of the tex
						fseek( pFile, 0, SEEK_END );
						nSize = ftell(pFile);
						rewind( pFile );
						fclose( pFile );
		
						if ( nSize > 1024 )
						{
							ModelConvTextAdd( "Model file - %s exported. (%dk)", acString2, (nSize/1024) );
						}
						else
						{
							ModelConvTextAdd( "Model file - %s exported. (%d bytes)", acString2, nSize );
						}
						UpdateWindow( mhwndMainDialog );
					}
					ModelFree( nLoadedModelHandle );
				}
				nProgressBar++;
				SendDlgItemMessage( hwndPopup, IDC_PROGRESS1, PBM_SETPOS, (WPARAM)nProgressBar, 0 );
				UpdateWindow( hwndPopup );
			}
	 
		    if (!FindNextFile(hSearch, &FileData)) 
		    {
	            fFinished = TRUE; 
		    }
		} 
		// Close the search handle.  
		FindClose(hSearch);
	}

	EndDialog( hwndPopup, 0 );
}

void ModelConvBatchRescaleRun( uint32 ulFlags )
{
//	ModelConvBatchRescaleProcessFolderNoRecurse( macBatchRescaleFolder );
	
	mhwndBatchRescalePopup = CreateDialog(ghInstance, (LPCTSTR)IDD_BATCH_PROGRESS, mhwndBatchRescaleDialog, (DLGPROC)PopupDlgProc );

	ModelConvBatchRescaleRecurseFolders( macBatchRescaleFolder );

	EndDialog( mhwndBatchRescalePopup, 0 );
}


void ModelConvBatchConversionRun( uint32 ulFlags )
{
WIN32_FIND_DATA FileData; 
char	acString[256];
char	acString2[256];
char	acFolder[256];
HANDLE hSearch; 
BOOL fFinished = FALSE; 
int		nLoadedModelHandle;
int		nNumModelFiles = 0;
int		nProgressBar;
HWND	hwndPopup;
char*	pcString;
FILE*	pFile;
int		nSize;
uint32	ulExportFlags = ModelConvGetExportFlags();

	hwndPopup = CreateDialog(ghInstance, (LPCTSTR)IDD_BATCH_PROGRESS, mhwndBatchConvertDialog, (DLGPROC)PopupDlgProc );

	SendDlgItemMessage( hwndPopup, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM( 0, 10 ) );
	SendDlgItemMessage( hwndPopup, IDC_PROGRESS1, PBM_SETPOS, (WPARAM)0, 0 );
	nProgressBar = 0;
    ShowWindow( hwndPopup, SW_SHOW );
	UpdateWindow( hwndPopup );

	strcpy( acFolder, macBatchConvFolder );
	pcString= &acFolder[ strlen( acFolder ) - 1 ];
	if ( *pcString != '\\' )
	{
		sprintf(acFolder , "%s/*.*", acFolder );
	}
	hSearch = FindFirstFile(acFolder, &FileData); 

	if (hSearch != INVALID_HANDLE_VALUE) 
	{ 
		while ( !fFinished )
		{ 
			pcString = &FileData.cFileName[ strlen( FileData.cFileName ) - 2 ];
			if ( stricmp( pcString, ".x" ) == 0 )
			{
				nNumModelFiles++;
			}
	 
		    if (!FindNextFile(hSearch, &FileData)) 
		    {
	            fFinished = TRUE; 
		    }
		} 
		// Close the search handle.  
		FindClose(hSearch);
	}
	SendDlgItemMessage( hwndPopup, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM( 0, nNumModelFiles ) );

	fFinished = FALSE;
	hSearch = FindFirstFile(acFolder, &FileData); 

	if (hSearch != INVALID_HANDLE_VALUE) 
	{ 
		while ( !fFinished )
		{ 
			pcString = &FileData.cFileName[ strlen( FileData.cFileName ) - 2 ];
			if ( stricmp( pcString, ".x" ) == 0 )
			{
				sprintf( acString, "Converting.. %s", FileData.cFileName );
				SetDlgItemText( hwndPopup, IDC_CONVERTING, acString );
				sprintf( acString2, "%s/%s", macBatchConvFolder, FileData.cFileName );
				nLoadedModelHandle = ModelConvLoadModel( acString2, 0, 0.0035f );
				if ( nLoadedModelHandle != NOTFOUND )
				{
					ModelConvTextAdd( "Loaded .x file - %s. (%dk)", acString2, ModelGetStats(nLoadedModelHandle)->nFileSize/1024 );
					UpdateWindow( mhwndMainDialog );

					switch( ulFlags )
					{
					case 2:
						ModelConvStickToFloor( nLoadedModelHandle );
						break;
					case 1:
						ModelConvFixBuildingScale( nLoadedModelHandle );
						break;
					default:
						break;
					}
					sprintf( acString2, "%s/%s", macBatchConvFolder, FileData.cFileName );
					sprintf( &acString2[ strlen( acString2 ) - 2 ], ".atm" );

					ModelExportATM( nLoadedModelHandle, acString2, ulExportFlags, 0 );
					pFile = fopen( acString2, "rb" );
					if ( pFile != NULL )
					{
						// Get the size of the tex
						fseek( pFile, 0, SEEK_END );
						nSize = ftell(pFile);
						rewind( pFile );
						fclose( pFile );
		
						if ( nSize > 1024 )
						{
							ModelConvTextAdd( "Model file - %s exported. (%dk)", acString2, (nSize/1024) );
						}
						else
						{
							ModelConvTextAdd( "Model file - %s exported. (%d bytes)", acString2, nSize );
						}
						UpdateWindow( mhwndMainDialog );
					}
					ModelFree( nLoadedModelHandle );
				}
				nProgressBar++;
				SendDlgItemMessage( hwndPopup, IDC_PROGRESS1, PBM_SETPOS, (WPARAM)nProgressBar, 0 );
				UpdateWindow( hwndPopup );

			}
	 
		    if (!FindNextFile(hSearch, &FileData)) 
		    {
	            fFinished = TRUE; 
		    }
		} 
		// Close the search handle.  
		FindClose(hSearch);
	}

	EndDialog( hwndPopup, 0 );
}


LRESULT CALLBACK PrelightDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
//int		nVal;

	switch (message)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage( hDlg, IDC_RADIO1, BM_SETCHECK, BST_CHECKED, 0 );
		SetDlgItemText( hDlg, IDC_EDIT3, (LPCTSTR)("80") );
		SetDlgItemText( hDlg, IDC_EDIT4, (LPCTSTR)("20") );
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDOK:
				ModelConvPreLight( m_MainSceneObject.GetModelHandle() );
				ModelConverterDisplayFrame( FALSE );
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


/***************************************************************************
 * Function    : PanicDlgProc
 * Params      :
 * Description :
 ***************************************************************************/
LRESULT CALLBACK PanicDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
//int		nVal;

	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
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


LRESULT CALLBACK BatchRescaleDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
//int		nVal;

	switch (message)
	{
	case WM_INITDIALOG:
		mhwndBatchRescaleDialog = hDlg;
		SetDlgItemText( hDlg, IDC_BATCH_FOLDER, macBatchRescaleFolder );
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case EN_CHANGE:
			{
//			char	acString[256];
//			float	fVal;

//				GetDlgItemText( mhwndMainDialog, IDC_CAM_DIST, acString, 256);
//				fVal = strtod( acString, NULL );
//				mfCamDist = fVal;
//				ModelConverterSetupCamera();
//				ModelConverterDisplayFrame();
			}
			break;
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDC_BUTTON1:
				ModelConvBrowseForFolder( macBatchRescaleFolder );
				SetDlgItemText( hDlg, IDC_BATCH_FOLDER, macBatchRescaleFolder );
//				ModelConvBrowseForFolder();
				break;
			case IDOK:
				GetDlgItemText( hDlg, IDC_BATCH_FOLDER, macBatchRescaleFolder, 256 );

				ModelConvBatchRescaleRun( 0 );
				EndDialog(hDlg, 0);
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



/***************************************************************************
 * Function    : BatchConvertDlgProc
 * Params      :
 * Returns     :
 * Description :
 ***************************************************************************/
LRESULT CALLBACK BatchConvertDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
int		nVal;

	switch (message)
	{
	case WM_INITDIALOG:
		mhwndBatchConvertDialog = hDlg;
		SetDlgItemText( hDlg, IDC_EDIT1, macBatchConvFolder );
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case EN_CHANGE:
			{
//			char	acString[256];
//			float	fVal;

//				GetDlgItemText( mhwndMainDialog, IDC_CAM_DIST, acString, 256);
//				fVal = strtod( acString, NULL );
//				mfCamDist = fVal;
//				ModelConverterSetupCamera();
//				ModelConverterDisplayFrame();
			}
			break;
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDC_BUTTON1:
				ModelConvBrowseForFolder( macBatchConvFolder );
				SetDlgItemText( mhwndBatchConvertDialog, IDC_EDIT1, macBatchConvFolder );
				break;
			case IDOK:
				GetDlgItemText( hDlg, IDC_EDIT1, macBatchConvFolder, 256 );

				nVal = SendDlgItemMessage( hDlg, IDC_BUILDING_SCALE_CHECK, BM_GETCHECK, 0, 0 );

				if ( nVal == BST_CHECKED )
				{
					ModelConvBatchConversionRun( 1 );
				}
				else
				{
					nVal = SendDlgItemMessage( hDlg, IDC_STICK_TO_FLOOR, BM_GETCHECK, 0, 0 );
					if ( nVal == BST_CHECKED )
					{
						ModelConvBatchConversionRun( 2 );
					}
					else
					{
						ModelConvBatchConversionRun( 0 );
					}
				}
				EndDialog(hDlg, 0);
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


void	ModelConvExportMaterialTexture( int nMaterialIndex, ModelMaterialData* pModelMaterial, char* acSourceModelFilename )
{
int		hTexture = pModelMaterial->GetTexture( DIFFUSEMAP );
					
	if ( hTexture != NOTFOUND )
	{
	char	acTextureOutName[256];
	char	acTempString[256];
	char*	pcPathRunner;

		strcpy( acTextureOutName, acSourceModelFilename );
		acTextureOutName[ strlen( acTextureOutName ) - 2 ] = 0;

		sprintf( acTempString, "_tex%04d.bmp", nMaterialIndex );
		strcat( acTextureOutName, acTempString );
		EngineExportTexture( hTexture, acTextureOutName, 0 );

		// TODO - Strip the path..
		pcPathRunner = acTextureOutName + strlen( acTextureOutName ) - 1;
		while ( pcPathRunner > acTextureOutName )
		{
			if ( ( *pcPathRunner == '\\' ) ||
				 ( *pcPathRunner == '/' ) )
			{
				pcPathRunner++;
				break;
			}
			pcPathRunner--;
		}
		pModelMaterial->SetFilename( DIFFUSEMAP, pcPathRunner );
	}

}

void	ModelConvExportX( int nModelHandle, char* acFilename )
{
MODEL_RENDER_DATA* pxModelData;
HRESULT		ret;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( pxModelData->xStats.nNumMaterials > 1 )
	{
	int		nNumMaterials = pxModelData->xStats.nNumMaterials;
	D3DXMATERIAL*			pMaterialExportMem = (D3DXMATERIAL*)( malloc( sizeof( D3DXMATERIAL ) * nNumMaterials ) );
	D3DXMATERIAL*			pMaterials = pMaterialExportMem;
	MATERIAL_COLOUR			matColour;
	ModelMaterialData*		pModelMaterial;
	int						nCount = 0;
		
		pModelMaterial = FindMaterial( pxModelData, nCount );
	
		while ( pModelMaterial )
		{
			// Ignore clones
			if ( pModelMaterial->GetCloneTextureMaterialIndex( 0 ) == NOTFOUND )
			{
				ModelConvExportMaterialTexture( pModelMaterial->GetAttrib(), pModelMaterial, acFilename );
				pMaterials->pTextureFilename = (LPSTR)( pModelMaterial->GetFilename( DIFFUSEMAP ) );
			}
			else
			{
			ModelMaterialData*		pCloneSourceMaterial = pxModelData->pMaterialData;
			int		nCloneFind = pModelMaterial->GetCloneTextureMaterialIndex( 0 );

				// If clone, we need to set the texture name to the same as the cloned version
				while ( ( pCloneSourceMaterial ) &&
					    ( pCloneSourceMaterial->GetAttrib() != nCloneFind ) )
				{
					pCloneSourceMaterial = pCloneSourceMaterial->GetNext();
				}

				if ( pCloneSourceMaterial )
				{
					// TODO - The thing we cloned didnt have a filename (yet)..
					ModelConvExportMaterialTexture( pCloneSourceMaterial->GetAttrib(), pCloneSourceMaterial, acFilename );
					pMaterials->pTextureFilename = (LPSTR)( pCloneSourceMaterial->GetFilename( DIFFUSEMAP ) );
				}
			}

			matColour = pxModelData->pMaterialData->GetColour( ModelMaterialData::AMBIENT );
			pMaterials->MatD3D.Ambient.a = matColour.fAlpha;
			pMaterials->MatD3D.Ambient.r = matColour.fRed;
			pMaterials->MatD3D.Ambient.g = matColour.fGreen;
			pMaterials->MatD3D.Ambient.b = matColour.fBlue;
			matColour = pxModelData->pMaterialData->GetColour( ModelMaterialData::SPECULAR );
			pMaterials->MatD3D.Specular.a = matColour.fAlpha;
			pMaterials->MatD3D.Specular.r = matColour.fRed;
			pMaterials->MatD3D.Specular.g = matColour.fGreen;
			pMaterials->MatD3D.Specular.b = matColour.fBlue;
			matColour = pxModelData->pMaterialData->GetColour( ModelMaterialData::EMISSIVE );
			pMaterials->MatD3D.Emissive.a = matColour.fAlpha;
			pMaterials->MatD3D.Emissive.r = matColour.fRed;
			pMaterials->MatD3D.Emissive.g = matColour.fGreen;
			pMaterials->MatD3D.Emissive.b = matColour.fBlue;
			matColour = pxModelData->pMaterialData->GetColour( ModelMaterialData::DIFFUSE );
			pMaterials->MatD3D.Diffuse.a = matColour.fAlpha;
			pMaterials->MatD3D.Diffuse.r = matColour.fRed;
			pMaterials->MatD3D.Diffuse.g = matColour.fGreen;
			pMaterials->MatD3D.Diffuse.b = matColour.fBlue;
			pMaterials->MatD3D.Power = pModelMaterial->GetSpecularPower();
			pMaterials++;

			nCount++;
			pModelMaterial = FindMaterial( pxModelData, nCount );
		}

		pMaterials = pMaterialExportMem;
		ret = D3DXSaveMeshToX( (LPSTR)acFilename, (LPD3DXMESH)pxModelData->pxBaseMesh->GetPlatformMeshImpl(), NULL, pMaterials, NULL, nNumMaterials,DXFILEFORMAT_TEXT );
	}
	else
	{
		ret = D3DXSaveMeshToX( (LPSTR)acFilename, (LPD3DXMESH)pxModelData->pxBaseMesh->GetPlatformMeshImpl(), NULL, NULL, NULL, 0, DXFILEFORMAT_TEXT );
	}

	if ( ret == D3D_OK )
	{
//		return( TRUE );
	}
//	return( FALSE );
}

void	ModelConvSaveSelection( void )
{
int			nMainModelHandle = m_MainSceneObject.GetModelHandle();
int			nNumFaces;
int			nNumVerts;
EngineMesh*		pxNewMesh;
int			nNewModelHandle;
MODEL_RENDER_DATA*	pxNewModelData;

	nNumFaces = m_MainSceneObject.GetNumFacesSelected();
	if ( nNumFaces > 0 )
	{
		nNumVerts = nNumFaces * 3;

		EngineCreateMesh( nNumFaces, nNumVerts, &pxNewMesh, 0 );
	
		// Copy all the selected faces & verts to the new model
		m_MainSceneObject.CopySelectedFaces( pxNewMesh );

		nNewModelHandle = ModelRenderGetNextHandle();
		if ( nNewModelHandle != NOTFOUND )
		{
			pxNewModelData = &maxModelRenderData[ nNewModelHandle ];
			pxNewModelData->bModelType = ASSETTYPE_STATIC_MESH;
			pxNewModelData->xGlobalProperties.bOpacity = 100;
			pxNewModelData->pxBaseMesh = pxNewMesh;
	//TODO			pxNewModelData->xStats.fBoundSphereRadius
	//todo			pxNewModelData->xStats.xBoundSphereCentre 
	//todo			pxNewModelData->xStats.xBoundMin 
	//todo			pxNewModelData->xStats.xBoundMin 
			pxNewModelData->xStats.nNumIndices = nNumFaces * 3;
			pxNewModelData->xStats.nNumMaterials = 0;
			pxNewModelData->xStats.nNumVertices = nNumFaces * 3;

			ModelConvSaveAsDialog( nNewModelHandle, FALSE );
			ModelFree( nNewModelHandle );				
		}
	}
	else
	{
		ModelConvSaveAsDialog(m_MainSceneObject.GetModelHandle(), TRUE );
	}

}

//---------------------------------------------------
// Function : ModelConvSaveAs
//		
//---------------------------------------------------
void	ModelConvSaveAs( int nModelHandle, char* acFilename, int nMode, BOOL bStoreSavedFilenameAsCurrent )
{
char	acString[256];
FILE*	pFile;
int		nSize;
uint32	ulFlags = ModelConvGetExportFlags();
uint32	ulLockFlags = 0;

	if ( ModelGetStats(nModelHandle)->ulLockID != 0 )
	{
		ulLockFlags = 1;
	}

	switch ( nMode )
	{
	case 0:
		ModelExportATM( nModelHandle, acFilename, ulFlags, ulLockFlags );
		break;
	case 1:
		ModelLoaderExport3ds( nModelHandle, acFilename );
		break;
	case 2:
		ModelConvExportX( nModelHandle, acFilename );
		break;
	case 3:
		ModelLoaderExportFBX( nModelHandle, acFilename );
		break;
	}

	pFile = fopen( acFilename, "rb" );
	if ( pFile != NULL )
	{
		// Get the size of the tex
		fseek( pFile, 0, SEEK_END );
		nSize = ftell(pFile);
		rewind( pFile );
		fclose( pFile );
		
		if ( bStoreSavedFilenameAsCurrent)
		{
			strcpy( maszCurrentModelFilename, acFilename );
		}
		ModelConvTextAdd( "Model file - %s exported. (%dk)", acFilename, (nSize/1024) );
		sprintf( acString, "Model Converter %s - %s", MODELCONV_VERSIONSTRING, acFilename );
		SendMessage( mhwndMainDialog, WM_SETTEXT, 0, (LPARAM)acString );
	}
	else
	{
		ModelConvTextAdd( "Model file could not be opened" );
	}

}


//---------------------------------------------------
// Function : ModelConvSave
//		
//---------------------------------------------------
void	ModelConvSave( void )
{
	ModelConvSaveAs( m_MainSceneObject.GetModelHandle(), maszCurrentModelFilename, 0, FALSE );
}


void	ModelConvCopyUVs( int hFromModel, int hToModel )
{
MODEL_RENDER_DATA*		pxFromModelData = &maxModelRenderData[hFromModel];
MODEL_RENDER_DATA*		pxToModelData = &maxModelRenderData[hToModel];
int		nNumVerts = pxFromModelData->xStats.nNumVertices;
int		nLoop;
CUSTOMVERTEX*	pxFromVerts;
CUSTOMVERTEX*	pxToVerts;

	if ( pxFromModelData->xStats.nNumVertices != pxToModelData->xStats.nNumVertices )
	{
		SysMessageBox( "Warning: Models do not have the same number of vertices", "Import UVs", 0 );
	}
	else
	{
		if ( pxToModelData->xStats.nNumVertices < nNumVerts )
		{
			nNumVerts = pxToModelData->xStats.nNumVertices;
		}
	}

	pxFromModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxFromVerts ) );
	pxToModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxToVerts ) );

	for( nLoop = 0; nLoop < nNumVerts; nLoop++ )
	{
		pxToVerts->tu = pxFromVerts->tu;
		pxToVerts->tv = pxFromVerts->tv;
		pxToVerts++;
		pxFromVerts++;
	}
	
	pxFromModelData->pxBaseMesh->UnlockVertexBuffer();
	pxToModelData->pxBaseMesh->UnlockVertexBuffer();
}


void	ModelConverterImportUVs( void )
{
char*		pszLoadFilter = "All supported formats\0*.x;*.3ds;*.atm;*.fbx;*.obj;*.bsp\0DirectX Model Files (*.x)\0*.x\03DS Files (*.3ds)\0*.3ds\0The Universal Model Format (*.atm)\0*.atm\0Autodesk FBX (*.fbx)\0*.fbx\0Wavefront OBJ Files (*.obj)\0*.obj\0Quake 2/Halflife BSP File (*.bsp)\0*.bsp\0All Files (*.*)\0*.*\0";
char		acFilename[256];
int			hImportModel;

	acFilename[0] = 0;

	SysGetOpenFilenameDialog( pszLoadFilter, "Select model file to Import UVs from..", macModelConvLastLoadFolder, 0, acFilename );

	if ( acFilename[0] != 0 )
	{
		GetPathFromFilename( acFilename, macModelConvLastLoadFolder );
		hImportModel = ModelLoad( acFilename, 0, 1.0f );

		if ( hImportModel != NOTFOUND )
		{
			ModelConvCopyUVs( hImportModel, m_MainSceneObject.GetModelHandle() );

			ModelFree( hImportModel );
			ModelConverterDisplayFrame( FALSE );
		}

		// Popup?
	}
}


//---------------------------------------------------
// Function : ModelConvSaveAsDialog
//		
//---------------------------------------------------
void	ModelConvSaveAsDialog( int nModelHandle, BOOL bStoreSavedFilenameAsCurrent )
{
OPENFILENAME	ofnOpen;
char		acFileName[256];
char		acCurrentDir[256];

//	sprintf( acFileName, macCurrentFilename );
	sprintf( acFileName, "" );
	GetCurrentDirectory( 256, acCurrentDir );

	ZeroMemory( &ofnOpen, sizeof( ofnOpen ) );
	ofnOpen.lStructSize       = sizeof(OPENFILENAME);
    ofnOpen.hwndOwner         = mhwndMainDialog;
    ofnOpen.hInstance         = 0;
	ofnOpen.lpstrFilter       = (LPSTR)"A Tractor Model File(*.atm)\0*.atm\0Autodesk 3ds File(*.3ds)\0*.3ds\0DirectX file(*.x)\0*.x\0FBX file(*.fbx)\0*.fbx\0";
    ofnOpen.lpstrCustomFilter = NULL;
    ofnOpen.nMaxCustFilter    = 0;
    ofnOpen.nFilterIndex      = 1;
    ofnOpen.lpstrFile         = (LPSTR)acFileName;
    ofnOpen.nMaxFile          = sizeof(acFileName);
    ofnOpen.lpstrInitialDir   = macModelConvLastSaveFolder;
    ofnOpen.lpstrTitle        = "Save Model File";
    ofnOpen.Flags             = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofnOpen.nFileOffset       = 0;
    ofnOpen.nFileExtension    = 0;

	if ( GetSaveFileName( &ofnOpen ) > 0 )
	{
		GetPathFromFilename( acFileName, macModelConvLastSaveFolder );
		ModelConvSaveUsedDirectories();

		switch( ofnOpen.nFilterIndex )
		{
		case 1:
		default:
			if ( stricmp( ".atm", &acFileName[strlen(acFileName)-4] ) != 0 )
			{
				sprintf( acFileName, "%s.atm", acFileName );
			}

			ModelConvSaveAs( nModelHandle, acFileName, 0, bStoreSavedFilenameAsCurrent);
			break;
		case 2:
			if ( stricmp( ".3ds", &acFileName[strlen(acFileName)-4] ) != 0 )
			{
				sprintf( acFileName, "%s.3ds", acFileName );
			}
			ModelConvSaveAs( nModelHandle, acFileName, 1, bStoreSavedFilenameAsCurrent);
			break;
		case 3:
			if ( stricmp( ".x", &acFileName[strlen(acFileName)-2] ) != 0 )
			{
				sprintf( acFileName, "%s.x", acFileName );
			}
			ModelConvSaveAs( nModelHandle, acFileName, 2, bStoreSavedFilenameAsCurrent);
			break;
		case 4:
			if ( stricmp( ".fbx", &acFileName[strlen(acFileName)-4] ) != 0 )
			{
				sprintf( acFileName, "%s.fbx", acFileName );
			}
			ModelConvSaveAs( nModelHandle, acFileName, 3, bStoreSavedFilenameAsCurrent);
			break;
		}

	}
}

void	ModelConvExportThumbnail( void )
{
char		acExportFilename[256];
char		acCurrentDir[256];

	acExportFilename[0] = 0;
	ModelConvGetLastLoadFolder( acCurrentDir );

	mbDisableWASD = TRUE;
	if ( SysGetSaveFilenameDialog( "PNG File(*.png)\0*.png\0", "Export Thumbnail", acCurrentDir, 0, acExportFilename ) == TRUE )
	{
	int		hThumbnailTarget = EngineCreateRenderTargetTexture( 600, 424, 3 );
	int		nHandleToDraw;
	MODEL_RENDER_DATA*	pxModelData;
	VECT	xPos;

		SysAddFileExtensionIfNeeded( acExportFilename, "png" );
		EngineSetRenderTargetTexture( hThumbnailTarget, 0x00000000, TRUE );

		ModelConverterSetupCamera();
		ModelConverterRenderShadowMapDepthPass();
		ModelConverterSetupCamera();

		nHandleToDraw = m_MainSceneObject.GetModelHandle();
		
		if ( nHandleToDraw  != NOTFOUND )
		{	
			pxModelData = &maxModelRenderData[ nHandleToDraw ];

			EngineSetTexture( 0, m_MainSceneObject.GetTextureHandle() );
			g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
			EngineEnableBlend( TRUE );
			EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
			EngineEnableAlphaTest( TRUE );
			EngineEnableZWrite( TRUE );
			EngineEnableZTest( TRUE );
			EngineEnableFog( FALSE );
			EngineEnableCulling( 1 );
			EngineEnableLighting( TRUE );
			EngineEnableSpecular(TRUE);
			EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );

			xPos.x = 0.0f;
			xPos.y = 0.0f;
			xPos.z = 0.0f;
			if ( EngineSceneShadowMapIsActive() )
			{
				EngineSceneShadowsStartRender( TRUE, FALSE, FALSE );
			}
			else
			{
				EngineSetShadowMultitexture( FALSE );
			}
			mnPolysRendered = m_MainSceneObject.Render(CSceneObject::NORMAL);

			if ( EngineSceneShadowMapIsActive() )
			{
				EngineSceneShadowsEndRender();
			}
		}

		EngineRestoreRenderTarget();
		EngineExportTexture( hThumbnailTarget, acExportFilename, 3 );
		EngineReleaseTexture( &hThumbnailTarget );
	}
	mbDisableWASD = FALSE;

}


TEXTURE_HANDLE		ModelConvGetOverrideTexture( void )
{
	return( m_MainSceneObject.GetTextureHandle() );
}


void	ModelConvClearTexture( void )
{
	m_MainSceneObject.ReleaseTexture( );
	ModifyMenu( GetMenu( mhwndMainDialog ), ID_RENDERING_CLEAROVERRIDETEXTURE, MF_GRAYED | MF_DISABLED, ID_RENDERING_CLEAROVERRIDETEXTURE, "Clear Override Texture" );		
}



void ModelConvSelectTexture( void )
{
OPENFILENAME	ofnOpen;
char		acFileName[256];
//char		acString[512];
char		acCurrentDir[256];
//int		nRet;
//D3DSURFACE_DESC		xSurfaceDesc;
int		nFileSize;
FILE*	pFile;

//	sprintf( acFileName, macCurrentFilename );
	sprintf( acFileName, "" );
	
	GetCurrentDirectory( 256, acCurrentDir );

	ZeroMemory( &ofnOpen, sizeof( ofnOpen ) );
	ofnOpen.lStructSize       = sizeof(OPENFILENAME);
    ofnOpen.hwndOwner         = mhwndMainDialog;
    ofnOpen.hInstance         = 0;
	ofnOpen.lpstrFilter       = (LPSTR)"Supported Image Formats (*.bmp,*.tga,*.jpg,*.dds,*.png)\0*.bmp;*.tga;*.jpg;*.dds;*.png\0All Files(*.*)\0*.*\0";
    ofnOpen.lpstrCustomFilter = NULL;
    ofnOpen.nMaxCustFilter    = 0;
    ofnOpen.nFilterIndex      = 1;
    ofnOpen.lpstrFile         = (LPSTR)acFileName;
    ofnOpen.nMaxFile          = sizeof(acFileName);
    ofnOpen.lpstrInitialDir   = NULL;
    ofnOpen.lpstrTitle        = "Select texture file";
    ofnOpen.Flags             = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofnOpen.nFileOffset       = 0;
    ofnOpen.nFileExtension    = 0;

	if ( GetOpenFileName( &ofnOpen ) > 0 )
	{
		pFile = fopen( acFileName, "rb" );
		if ( pFile != NULL )
		{
			fseek( pFile, 0, SEEK_END );
			nFileSize = ftell(pFile);
			fclose( pFile );

			ModelConvLoadMainRenderTexture( acFileName );
		}	
	}
	SetCurrentDirectory( acCurrentDir );
	ModelConverterDisplayFrame( FALSE );

	ModifyMenu( GetMenu( mhwndMainDialog ), ID_RENDERING_CLEAROVERRIDETEXTURE, MF_ENABLED, ID_RENDERING_CLEAROVERRIDETEXTURE, "Clear Override Texture" );		

}



//-----------------------------------------------------------------------------------------------------
class ModelConverterEngineGameInterface : public EngineGameInterface
{
public:
	virtual VECT*			GetPlayerPosition( void );
	virtual int				GetUniqueMachineID( void );
	virtual bool			GetFogSetting( void );				// Temp - should be internalised
	virtual void			GetShadowMatrix( ENGINEMATRIX* pxMatrix, VECT* pxOffset );		// Temp - should be internalised
	virtual void			ShadowMapCopyRenderToShadowMap( const VECT*	pxPos );			// Temp - should be internalised

	virtual void			AddParticle( int nType, VECT* pxPos );
	virtual void			AddLight( VECT* pxPos, float, float, float, float R, float G, float B );

private:
	VECT			mxNullPos;
};


VECT*		ModelConverterEngineGameInterface::GetPlayerPosition( void )
{
	return( &mxNullPos );
}
int		ModelConverterEngineGameInterface::GetUniqueMachineID( void )
{
	return( 1 );
}
bool	ModelConverterEngineGameInterface::GetFogSetting( void ) // Temp - should be internalised to engine
{
	return( false );
}
void	ModelConverterEngineGameInterface::GetShadowMatrix( ENGINEMATRIX* pxMatrix, VECT* pxOffset )		// Temp - should be internalised to engine
{
//	GameGetShadowMatrix( pxMatrix, pxOffset );
}
void	ModelConverterEngineGameInterface::ShadowMapCopyRenderToShadowMap( const VECT*	pxPos )			// Temp - should be internalised to engine
{
//	GameShadowMapCopyRenderToShadowMap( pxPos );
}

void	ModelConverterEngineGameInterface::AddParticle( int nType, VECT* pxPos )
{
	RenderEffectsAddParticle( nType, pxPos );
	
}

void	ModelConverterEngineGameInterface::AddLight( VECT* pxPos, float atten0, float atten1, float atten2, float R, float G, float B )
{
//	IslandLightingAddBulb( pxPos, atten0, atten1, atten2, R, G, B );
}


ModelConverterEngineGameInterface		m_sEngineGameInterface;


/***************************************************************************
 * Function    : ModelConverterMainDlgProc
 * Params      :
 * Returns     :
 * Description : Message handler for the "save changes" dialog box
 ***************************************************************************/
void ModelConverterInitGraphicWindow( void)
{
RECT		xWindowRect;
WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, ModelConverterGraphicWindowDlgProc, 0L, 0L,GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "ModelConv", NULL };
WNDCLASSEX*	pWinClass = &wc;
DWORD		dwWindowStyle;
TCITEM	xTCItem;
char	acString[256];
int		nLoop;

	sprintf( acString, "3d View" );
	xTCItem.mask = TCIF_TEXT;
	xTCItem.pszText = acString;
	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_INSERTITEM, 0, (LPARAM)( &xTCItem ) );

	sprintf( acString, "Left" );
	xTCItem.mask = TCIF_TEXT;
	xTCItem.pszText = acString;
	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_INSERTITEM, 1, (LPARAM)( &xTCItem ) );
	sprintf( acString, "Front" );
	xTCItem.mask = TCIF_TEXT;
	xTCItem.pszText = acString;
	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_INSERTITEM, 2, (LPARAM)( &xTCItem ) );
	sprintf( acString, "Right" );
	xTCItem.mask = TCIF_TEXT;
	xTCItem.pszText = acString;
	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_INSERTITEM, 3, (LPARAM)( &xTCItem ) );
	sprintf( acString, "Back" );
	xTCItem.mask = TCIF_TEXT;
	xTCItem.pszText = acString;
	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_INSERTITEM, 4, (LPARAM)( &xTCItem ) );
	sprintf( acString, "Top" );
	xTCItem.mask = TCIF_TEXT;
	xTCItem.pszText = acString;
	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_INSERTITEM, 5, (LPARAM)( &xTCItem ) );
	sprintf( acString, "Bottom" );
	xTCItem.mask = TCIF_TEXT;
	xTCItem.pszText = acString;
	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_INSERTITEM, 6, (LPARAM)( &xTCItem ) );

	SendDlgItemMessage( mhwndMainDialog, IDC_TAB1, TCM_SETCURSEL , 0, 0 );

	RegisterClassEx( pWinClass );

	InterfaceSetInitialSize( FALSE, 0, 0, FALSE );
	dwWindowStyle = WS_CHILD;
	xWindowRect.left = 136;
	xWindowRect.right = 705;
	xWindowRect.bottom = 415;
	xWindowRect.top = 17;
	AdjustWindowRect( &xWindowRect, dwWindowStyle, FALSE );
    // Create the application's window
     mhwndGraphicWindow = CreateWindowEx( WS_EX_ACCEPTFILES, "ModelConv", "A Tractor Model Converter",
                              dwWindowStyle, xWindowRect.left, xWindowRect.top,
						      xWindowRect.right - xWindowRect.left, xWindowRect.bottom - xWindowRect.top,
                              mhwndMainDialog, NULL, ghInstance, NULL );
	InterfaceSetWindow( mhwndGraphicWindow );
	g_pd3dDevice = InterfaceInitD3D(TRUE);
	EngineInitFromInterface();
	ModelConverterAddStandardLighting();
	
	EngineSetGameInterface( (EngineGameInterface*)&m_sEngineGameInterface );

	InterfaceSetGlobalParam( INTF_TEXTURE_FILTERING, 1 );

	InterfaceInit( TRUE );
	InterfaceNewFrame( 0 );
	InterfacePresent();
	ShowWindow( mhwndGraphicWindow, SW_SHOW );
	UpdateWindow( mhwndGraphicWindow );

	ModelRenderingInit();

	InitialiseLineRenderer(32000);
	InitialisePointRenderer();
	Sprites3DInitialise();
	RenderEffectsInit();

	ModelConverterSetupCamera();

	mnBallModelHandle = ModelLoad( "Ball1.atm", 0, 0.05f );
	if ( mnBallModelHandle != NOTFOUND )
	{
		ModelSetVertexColours( mnBallModelHandle, 0xA02020f0 );
	}

	SendDlgItemMessage( mhwndMainDialog, IDC_COMBO1, CB_RESETCONTENT, 0, 0 );
//	sprintf( acString, "<All>", nLoop );
//	SendDlgItemMessage( mhwndMainDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)acString );
	for ( nLoop = 0; nLoop < MAX_ANIMATION_USES; nLoop++ )
	{
		SendDlgItemMessage( mhwndMainDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)maszAnimationUses[nLoop] );
	}
	SendDlgItemMessage( mhwndMainDialog, IDC_COMBO1, CB_SETCURSEL, 0, 0 );

	SendDlgItemMessage( mhwndMainDialog, IDC_MAINLODCOMBO, CB_RESETCONTENT, 0, 0 );
	SendDlgItemMessage( mhwndMainDialog, IDC_MAINLODCOMBO, CB_ADDSTRING, 0, (LPARAM)"<Automatic>" );
	SendDlgItemMessage( mhwndMainDialog, IDC_MAINLODCOMBO, CB_ADDSTRING, 0, (LPARAM)"High" );
	SendDlgItemMessage( mhwndMainDialog, IDC_MAINLODCOMBO, CB_ADDSTRING, 0, (LPARAM)"Med" );
	SendDlgItemMessage( mhwndMainDialog, IDC_MAINLODCOMBO, CB_ADDSTRING, 0, (LPARAM)"Low" );
	SendDlgItemMessage( mhwndMainDialog, IDC_MAINLODCOMBO, CB_SETCURSEL, 0, 0 );

	SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_CAMERA, BM_SETCHECK, BST_CHECKED, 0 );
	SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_PICKER, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_VERTEXPICKER, BM_SETCHECK, BST_UNCHECKED, 0 );
	SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_POLYSLICE, BM_SETCHECK, BST_UNCHECKED, 0 );
	

	ModelConvShowSelectionWindow(0);

	PlaneCutterInit();

}


void	ModelConvRepositionLowerToolbar( int nWidth, int nHeight )
{
int		nXYZPositionOffsetX = 182;
int		nXYZPositionOffsetY = 132;
int		nToolbarOffsetY = 112;
int		nToolbarY = nHeight - nToolbarOffsetY;

	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_POSLABELX ), NULL, nWidth-nXYZPositionOffsetX, nHeight - nXYZPositionOffsetY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_POSVALUEX ), NULL, nWidth-(nXYZPositionOffsetX-10), nHeight - nXYZPositionOffsetY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);

	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_POSLABELY ), NULL, 60+nWidth-nXYZPositionOffsetX, nHeight - nXYZPositionOffsetY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_POSVALUEY ), NULL, 60+nWidth-(nXYZPositionOffsetX-10), nHeight - nXYZPositionOffsetY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_POSLABELZ ), NULL, 120+nWidth-nXYZPositionOffsetX, nHeight - nXYZPositionOffsetY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_POSVALUEZ ), NULL, 120+nWidth-(nXYZPositionOffsetX-10), nHeight - nXYZPositionOffsetY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
 
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_VIEWLODTEXT ), NULL, 132, nToolbarY + 4, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_MAINLODCOMBO ), NULL, 180, nToolbarY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
 
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_ANIMTEXT ), NULL, 300, nToolbarY + 4, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_COMBO1 ), NULL, 327, nToolbarY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);

	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_ANIM_STOP ), NULL, 455, nToolbarY + 2, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_ANIM_PLAY ), NULL, 505, nToolbarY + 2, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);

	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_REFRESH_TEXTURE ), NULL, nWidth - 142, nToolbarY + 2, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);

	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_VIEWLODTEXT2 ), NULL, 555, nToolbarY + 2, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_HORIZTURRET_ROT ), NULL, 600, nToolbarY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_VIEWLODTEXT3 ), NULL, 700, nToolbarY + 2, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_VERTTURRET_ROT ), NULL, 745, nToolbarY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	
}


void	ModelConvResizeWindow( int nWidth, int nHeight )
{
int		nLineY = nHeight - 87;

	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_OUTPUT_LIST ), NULL, 135, nHeight - 87, nWidth - 300, 80, SWP_SHOWWINDOW | SWP_NOZORDER );
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_TAB1 ), NULL, 0, 0, nWidth-135, nHeight-126, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOMOVE);
//	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_EDIT1 ), NULL, 135, nHeight - 28, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
//	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_RIGHT_TEXTBOX ), NULL, nWidth-100, nHeight - 87, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_MAIN_PROGRESS ), NULL, 10, nHeight - 20, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);

	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_RESET_CAM_TO_MODEL ), NULL, nWidth - 140, nLineY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_RESET_CAM_TO_MODEL ), TRUE );
	nLineY += 20;

	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_RESET_CAM_TO_SCENE ), NULL, nWidth - 140, nLineY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_RESET_CAM_TO_SCENE ), TRUE );
	nLineY += 22;

	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_LOCKTEXT ), NULL, nWidth - 140, nLineY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_LOCKX ), NULL, nWidth - 100, nLineY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_LOCKY ), NULL, nWidth - 70, nLineY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);
	SetWindowPos( GetDlgItem( mhwndMainDialog, IDC_LOCKZ ), NULL, nWidth - 40, nLineY, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOSIZE);

	SetWindowPos( mhwndGraphicWindow, NULL, 0, 0, nWidth-146, nHeight-160, SWP_SHOWWINDOW | SWP_NOZORDER |SWP_NOMOVE);
	InterfaceSetWindowSize( FALSE, nWidth-146, nHeight-155, TRUE );

	ModelConvRepositionLowerToolbar( nWidth, nHeight );
	UpdateWindow( mhwndMainDialog);
}
 

void ModelConvSetModelRenderingOption( int mode )
{
	mnModelRenderingMode = mode;
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_MODELDISPLAY_NORMAL, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_MODELDISPLAY_POLYEDGES, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_MODELDISPLAY_SHOWVERTICES, MF_UNCHECKED);

	switch( mode )
	{
	case 0:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_MODELDISPLAY_NORMAL, MF_CHECKED);
		break;
	case 1:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_MODELDISPLAY_POLYEDGES, MF_CHECKED);
		break;
	case 2:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_MODELDISPLAY_SHOWVERTICES, MF_CHECKED);
		break;
	}
}

void ModelConvSetLODDisplayOption( int mode )
{
	ModelRenderLODOverride( m_MainSceneObject.GetModelHandle(), mode );

	CheckMenuItem(GetMenu(mhwndMainDialog), ID_LEVELOFDETAIL_DISPLAY_DEPENDONVIEW, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_LEVELOFDETAIL_DISPLAY_HIGH, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_LEVELOFDETAIL_DISPLAY_MEDLODALWAYS, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_LEVELOFDETAIL_DISPLAY_LOWLODALWAYS, MF_UNCHECKED);

	switch( mode )
	{
	case 0:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_LEVELOFDETAIL_DISPLAY_DEPENDONVIEW, MF_CHECKED);
		break;
	case 1:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_LEVELOFDETAIL_DISPLAY_HIGH, MF_CHECKED);
		break;
	case 2:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_LEVELOFDETAIL_DISPLAY_MEDLODALWAYS, MF_CHECKED);
		break;
	case 3:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_LEVELOFDETAIL_DISPLAY_LOWLODALWAYS, MF_CHECKED);
		break;
	}
}



void	ModelConvSetBackgroundColourOption( int nMode )
{
	mnBackgroundColour = nMode;
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_GREY, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_GREEN, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_BLACK, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_WHITE, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_BLUE, MF_UNCHECKED);
	CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_RED, MF_UNCHECKED);
	switch( mnBackgroundColour )
	{
	case 0:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_GREY, MF_CHECKED);
		break;
	case 1:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_GREEN, MF_CHECKED);
		break;
	case 2:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_BLACK, MF_CHECKED);
		break;
	case 3:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_WHITE, MF_CHECKED);
		break;
	case 4:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_BLUE, MF_CHECKED);
		break;
	case 5:
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_BACKGROUNDCOLOUR_RED, MF_CHECKED);
		break;
	}

}




void	ModelConvAssignMaterialToSelectedFaces( void )
{
	DialogBox(ghInstance, (LPCTSTR)IDD_MATERIAL_ASSIGN, mhwndMainDialog, (DLGPROC)ModelConverterAssignMaterialDlgProc );
}




void	ModelConvSubdivideSelectedFaces( void )
{
	// TODO

}

void	ModelConvStartUVUnwrap( void )
{
int		nAttribID = m_MainSceneObject.GetSelectionMaterial();

	if ( nAttribID == -2 )
	{
		SysMessageBox( "Selection consists of multiple materials. Only 1 material UV map can be edited at a time", "Unable to use UV edit", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
	}
	else
	{
		UVUnwrapWindowInit(&m_MainSceneObject, nAttribID, mhwndMainDialog );
	}
}

void	ModelConvSetPickerMode( int mode )
{
	PlaneCutterActivate( FALSE );

	switch( mode )
	{
	case 0:
		mnPickerControlMode = CONTROL_CAMERA;
		SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_VERTEXPICKER, BM_SETCHECK, BST_UNCHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_PICKER, BM_SETCHECK, BST_UNCHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_CAMERA, BM_SETCHECK, BST_CHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog,IDC_CONTROL_POLYSLICE, BM_SETCHECK, BST_UNCHECKED, 0 );
		ModelConvShowSelectionWindow( 0 );
		break;
	case 1:
		mnPickerControlMode = CONTROL_PICKER;
		SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_VERTEXPICKER, BM_SETCHECK, BST_UNCHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_CAMERA, BM_SETCHECK, BST_UNCHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_PICKER, BM_SETCHECK, BST_CHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog,IDC_CONTROL_POLYSLICE, BM_SETCHECK, BST_UNCHECKED, 0 );
		ModelConvShowSelectionWindow( 1 );
		break;
	case 2:
		mnPickerControlMode = CONTROL_VERTEXPICKER;
		SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_CAMERA, BM_SETCHECK, BST_UNCHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_PICKER, BM_SETCHECK, BST_UNCHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog,IDC_CONTROL_VERTEXPICKER, BM_SETCHECK, BST_CHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog,IDC_CONTROL_POLYSLICE, BM_SETCHECK, BST_UNCHECKED, 0 );
		ModelConvShowSelectionWindow( 3 );
		break;
	case 3:
		mnPickerControlMode = CONTROL_POLYSLICE;
		SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_CAMERA, BM_SETCHECK, BST_UNCHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog, IDC_CONTROL_PICKER, BM_SETCHECK, BST_UNCHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog,IDC_CONTROL_VERTEXPICKER, BM_SETCHECK, BST_UNCHECKED, 0 );
		SendDlgItemMessage( mhwndMainDialog,IDC_CONTROL_POLYSLICE, BM_SETCHECK, BST_CHECKED, 0 );
		ModelConvShowSelectionWindow( 2 );
		PlaneCutterActivate( TRUE );
		break;
		
	}
}

void	ModelConvInvertSelection( void )
{
	m_MainSceneObject.InvertSelection();
}


void	ModelConvSelectAllConnectedFaces( int nSelectedFaceNum )
{
BYTE*	pbFaceConnections;
int		nModelHandle = m_MainSceneObject.GetModelHandle();
MODEL_RENDER_DATA* pxModelData = maxModelRenderData + nModelHandle;
int		nNumFaces = (pxModelData->xStats.nNumIndices/3);
VECT*	pxConnectedVertPositions;
VECT*	pxNextConnectedVertSlot;
int		nNumConnectedVerts = 0;
int		nNumConnectedFaces = 0;
int		nLastNumConnectedFaces = 0;
int		nFaceLoop;
CUSTOMVERTEX*		pxVertices;
ushort*		puwIndices;
//unsigned int*	punIndices;
unsigned int	aunFaceIndices[3];
int		nVertCheckLoop;
int		nFaceVertLoop;
VECT	xCheckPos;
BOOL	bFaceConnected = FALSE;

	pbFaceConnections = (BYTE*)malloc( nNumFaces * sizeof(BYTE) );
	memset( pbFaceConnections, 0, nNumFaces * sizeof(BYTE) );
	
	pxConnectedVertPositions = (VECT*)( malloc( pxModelData->xStats.nNumVertices * sizeof( VECT ) ) );
	memset( pxConnectedVertPositions, 0, pxModelData->xStats.nNumVertices * sizeof(VECT) );
	pxNextConnectedVertSlot = pxConnectedVertPositions;

	pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
	pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
			
	// TODO Add the 3 vert positions of the selected face to the pxNextConnectedVertSlot
	pbFaceConnections[nSelectedFaceNum] = 1;
	nNumConnectedVerts = 3;
	nNumConnectedFaces = 1;

	aunFaceIndices[0] = puwIndices[nSelectedFaceNum*3];
	aunFaceIndices[1] = puwIndices[(nSelectedFaceNum*3)+1];
	aunFaceIndices[2] = puwIndices[(nSelectedFaceNum*3)+2];

	pxConnectedVertPositions[0] = pxVertices[ aunFaceIndices[0] ].position;
	pxConnectedVertPositions[1] = pxVertices[ aunFaceIndices[1] ].position;
	pxConnectedVertPositions[2] = pxVertices[ aunFaceIndices[2] ].position;

	// check through the vertex positions for all faces, and if any match any of the vertex positions of the selected face 
	// (or match any other face already marked as connected) then flag them as connected
	while( nLastNumConnectedFaces != nNumConnectedFaces )
	{
		nLastNumConnectedFaces = nNumConnectedFaces;
		for ( nFaceLoop = 0; nFaceLoop < nNumFaces; nFaceLoop++ )
		{
			if ( pbFaceConnections[nFaceLoop] == 0 )
			{
				bFaceConnected = FALSE;
				aunFaceIndices[0] = puwIndices[nFaceLoop*3];
				aunFaceIndices[1] = puwIndices[(nFaceLoop*3)+1];
				aunFaceIndices[2] = puwIndices[(nFaceLoop*3)+2];

				for ( nFaceVertLoop = 0; nFaceVertLoop < 3; nFaceVertLoop++ )
				{
					xCheckPos = pxVertices[ aunFaceIndices[nFaceVertLoop] ].position;

					for ( nVertCheckLoop = 0; nVertCheckLoop < nNumConnectedVerts; nVertCheckLoop++ )
					{
						if ( ( xCheckPos.x == pxConnectedVertPositions[nVertCheckLoop].x ) &&
							 ( xCheckPos.y == pxConnectedVertPositions[nVertCheckLoop].y ) &&
							 ( xCheckPos.z == pxConnectedVertPositions[nVertCheckLoop].z ) )
						{
							bFaceConnected = TRUE;
							break;
						}
					}
					if ( bFaceConnected )
					{
						break;
					}
				}

				if ( bFaceConnected )
				{
					pbFaceConnections[nFaceLoop] = 1;
					pxConnectedVertPositions[nNumConnectedVerts] = pxVertices[ aunFaceIndices[0] ].position;
					pxConnectedVertPositions[nNumConnectedVerts+1] = pxVertices[ aunFaceIndices[1] ].position;
					pxConnectedVertPositions[nNumConnectedVerts+2] = pxVertices[ aunFaceIndices[2] ].position;
					nNumConnectedVerts += 3;
					nNumConnectedFaces++;
				}
			}
		}
	}

	m_MainSceneObject.SetSelectedFaces( pbFaceConnections );
	ModelConvTextAdd( "%d connected faces selected", nNumConnectedFaces );
}

void	ModelConvSelectAllWithSameMaterial( int nHandle )
{


}

void	ModelConvRemoveAllAnimations()
{
int			nModelHandle = m_MainSceneObject.GetModelHandle();
MODEL_RENDER_DATA* pxModelData = maxModelRenderData + nModelHandle;
int			nLoop;
CUSTOMVERTEX* pxVertices;

	if ( pxModelData->pxVertexKeyframes != NULL )
	{
		SystemFree( pxModelData->pxVertexKeyframes );
		pxModelData->pxVertexKeyframes = NULL;
	}
	if ( pxModelData->pxNormalKeyframes != NULL )
	{
		SystemFree( pxModelData->pxNormalKeyframes );
		pxModelData->pxNormalKeyframes = NULL;
	}
	pxModelData->xStats.nNumVertKeyframes = 0;

	for( nLoop = 0; nLoop < MAX_KEYFRAMES_IN_MODEL; nLoop++ )
	{
		pxModelData->axKeyframeData[nLoop].bAnimationUse = 0;
		pxModelData->axKeyframeData[nLoop].uwKeyframeTime = 0;
		pxModelData->axKeyframeData[nLoop].bAnimationTriggerCode = 0;

	}

	if ( pxModelData->pxBaseVertices )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
		{
			pxVertices[nLoop].position = pxModelData->pxBaseVertices[nLoop];
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
}


/***************************************************************************
 * Function    : ModelConverterMainDlgProc
 * Params      :
 * Returns     :
 * Description : Message handler for the "save changes" dialog box
 ***************************************************************************/
LRESULT CALLBACK ModelConverterMainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
LPNMHDR		pNotifyHeader;
int	nVal;

	switch (message)
	{
	case WM_DROPFILES:
		{
		char	acFilename[512];
		
			DragQueryFile( (HDROP)wParam, 0, acFilename, 512 );
			ModelConverterDropFile( acFilename );
		}
		break;
	case WM_ACTIVATEAPP:
		if ( wParam == FALSE )
		{
			mbThisWindowIsFocused = FALSE;
		}
		else
		{
			mbThisWindowIsFocused = TRUE;
		}
		break;
	case WM_HSCROLL:
		if ( LOWORD( wParam )== SB_THUMBTRACK )
		{
			if ( lParam == (LPARAM)GetDlgItem( mhwndMainDialog, IDC_HORIZTURRET_ROT ) )
			{
			float	fAngle = ((HIWORD(wParam))*A90)/100;

				ModelSetHorizTurretRotation( m_MainSceneObject.GetModelHandle(), fAngle );
			}
			else if ( lParam == (LPARAM)GetDlgItem( mhwndMainDialog, IDC_VERTTURRET_ROT ) )
			{
			float	fAngle = ((HIWORD(wParam))*A90)/100;

				ModelSetVertTurretRotation( m_MainSceneObject.GetModelHandle(), fAngle );
			}
		}
		break;
	case WM_TIMER:
		if ( mboViewIsAnimation == TRUE )
		{
			ModelConverterDisplayFrame( FALSE );
		}
		SetTimer( mhwndMainDialog, 1, 40, NULL );
		break;
	case WM_NOTIFY:
		pNotifyHeader = (LPNMHDR) lParam; 
		wNotifyCode = pNotifyHeader->code;
		switch ( wNotifyCode )
		{
		case TCN_SELCHANGE:
			mnViewPage = SendDlgItemMessage( hDlg, IDC_TAB1, TCM_GETCURSEL , 0, 0 );
			ModelConverterDisplayFrame( FALSE );
			break;
		}
		break;
	case WM_INITDIALOG:
		mhwndMainDialog = hDlg;
		SysSetMainWindow( (void*)hDlg );
		ModelConvLoadExportFlags();
		ModelConverterInitGraphicWindow();
		ModelConvResetCamera();
		{
		char acString[256];

			sprintf( acString, "Model Converter %s", MODELCONV_VERSIONSTRING );
			SendMessage( mhwndMainDialog, WM_SETTEXT, 0, (LPARAM)acString );
		}

		CheckMenuItem(GetMenu(mhwndMainDialog), IDM_DRAW_GRID_LINES, MF_CHECKED);
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_GOURAUDSHADING, MF_CHECKED);
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_CULLING_NORMAL, MF_CHECKED);
		CheckMenuItem(GetMenu(mhwndMainDialog), ID_ANIMATIONS_BLENDING_LINEAR, MF_CHECKED);	
		ModelConvSetExportFlags( mulLastExportFlags );
		SetTimer( mhwndMainDialog, 1, 50, NULL );
		if ( mxConfig.nHeight != 0 )
		{
			SetWindowPos( mhwndMainDialog, NULL, mxConfig.nX, mxConfig.nY, mxConfig.nWidth, mxConfig.nHeight, SWP_NOZORDER  );
		}

		if ( mszCommandLineFile[0] != 0 )
		{
			ModelConvLoadStart( kMAIN, mszCommandLineFile );		
			ModelConverterDisplayFrame( FALSE );
		}
		return TRUE;
	case WM_CHAR:					// None of these seem to be getting triggered atm? (Possibly coz the graphic subwindow  is stealing all the input?
		switch ( (short)( wParam ) )
		{
		case 'M':
		case 'm':
			mnPickerControlMode = CONTROL_CAMERA;
			break;
		case 'P':
		case 'p':
			mnPickerControlMode = CONTROL_PICKER;
			break;
		case 'V':
		case 'v':
			ModelConvResetCamera();
			ModelConverterSetupCamera();
			ModelConverterDisplayFrame( FALSE );
			break;
		}
		break;
	case WM_KEYDOWN:			// None of these seem to be getting triggered atm?  (Possibly coz the graphic subwindow is stealing all the input?)
		switch ( (short)( wParam ) )
		{
		case VK_F5:
			ModelConvReloadTexture();
			break;
		}
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case CBN_SELCHANGE:
			switch( LOWORD(wParam) )
			{
			case IDC_MAINLODCOMBO:
				nVal = SendDlgItemMessage( hDlg, IDC_MAINLODCOMBO, CB_GETCURSEL, 0, 0 );
				ModelRenderLODOverride( m_MainSceneObject.GetModelHandle(), nVal );
				break;
			case IDC_COMBO1:
				nVal = SendDlgItemMessage( hDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0 );
				ModelSetAnimationImmediate( m_MainSceneObject.GetModelHandle(), nVal, 100, NULL, 0 );
				break;
			}
			break;

		case EN_CHANGE:
			{
//			char	acString[256];
//			float	fVal;

//				GetDlgItemText( mhwndMainDialog, IDC_CAM_DIST, acString, 256);
//				fVal = strtod( acString, NULL );
//				mfCamDist = fVal;
//				ModelConverterSetupCamera();
//				ModelConverterDisplayFrame();
			}
			break;
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDC_SLICE_FACES:
				PlaneCutterSlice( &m_MainSceneObject );
				break;
			case IDC_CENTRE_BUT:
				ModelConvResetCamera();
				ModelConverterSetupCamera();
				ModelConverterDisplayFrame( FALSE );
				break;
			case ID_SPECIAL_MATERIALPROPERTIES:
				MaterialsDialogActivate();
				break;			
			case ID_OPTIONS_EXPORTSETTINGS:
				DialogBox(ghInstance, (LPCTSTR)IDD_EXPORT_SETTINGS, hDlg, (DLGPROC)ExportSettingsMainDlgProc );
				break;
			case IDC_ANIM_STOP:
				ModelAnimationPause( m_MainSceneObject.GetModelHandle(), TRUE );
				break;
			case IDC_ANIM_PLAY:
				ModelAnimationPause( m_MainSceneObject.GetModelHandle(), FALSE );
				break;
			case ID_EXPORTOPTIONS_MATERIALS:
			case ID_EXPORTOPTIONS_EFFECTS:
			case ID_EXPORTOPTIONS_COMPRESSTEXTURECOORDS:
			case ID_TOOLS_COMPRESSNORMALS:
			case ID_TOOLS_COMPRESSVERTICES:
			case ID_EXPORTOPTIONS_EXPORTVERTEXCOLOURS:
			case IDM_ATM_COMPRESSION:
				if ( (GetMenuState( GetMenu(mhwndMainDialog), LOWORD(wParam), 0 ) & MF_CHECKED) != 0 )
				{
					CheckMenuItem(GetMenu(mhwndMainDialog), LOWORD(wParam), MF_UNCHECKED);
				}
				else
				{
					CheckMenuItem(GetMenu(mhwndMainDialog), LOWORD(wParam), MF_CHECKED);
				}			
				mulLastExportFlags = ModelConvGetExportFlags();
				ModelConvSaveExportFlags();
				break;
			case ID_MATERIALS_DELETEALLMATERIALS:
				MaterialBrowserDeleteAllMaterials();
				ModifyMenu( GetMenu( mhwndMainDialog ), ID_MATERIALS_MATERIALBROWSER, MF_GRAYED | MF_DISABLED, ID_MATERIALS_MATERIALBROWSER, "Material Browser" );				
				ModifyMenu( GetMenu( mhwndMainDialog ), ID_MATERIALS_ADDMATERIALS, MF_ENABLED, ID_MATERIALS_ADDMATERIALS, "Create Materials" );		
				break;
			case ID_MATERIALS_COMBINEINTOSINGLEMATERIAL:
				ModelConvReduceToSingleMaterial( m_MainSceneObject.GetModelHandle() );
				break;
			case ID_MATERIALS_ADDMATERIALS:
				{
				MaterialBrowserAddDefaultMaterials();
				ModifyMenu( GetMenu( mhwndMainDialog ), ID_MATERIALS_MATERIALBROWSER, MF_ENABLED, ID_MATERIALS_MATERIALBROWSER, "Material Browser" );				
				ModifyMenu( GetMenu( mhwndMainDialog ), ID_MATERIALS_ADDMATERIALS, MF_GRAYED | MF_DISABLED, ID_MATERIALS_ADDMATERIALS, "Create Materials" );		
				HWND hMaterialDlg = CreateDialog(ghInstance, (LPCTSTR)IDD_MATERIAL_BROWSER, hDlg, (DLGPROC)MaterialBrowserDlgProc );
			    ShowWindow( hMaterialDlg, SW_SHOW );
				UpdateWindow( hMaterialDlg );
				}
				break;
			case ID_MATERIALS_MATERIALBROWSER:
				{
				HWND hMaterialDlg = CreateDialog(ghInstance, (LPCTSTR)IDD_MATERIAL_BROWSER, hDlg, (DLGPROC)MaterialBrowserDlgProc );
				    ShowWindow( hMaterialDlg, SW_SHOW );
					UpdateWindow( hMaterialDlg );
				}
				break;
			case ID_REMOVEALLANIMATIONS:
				ModelConvRemoveAllAnimations();
				break;
			case ID_ANIMATIONS_BUILDANIMATIONS:
				DialogBox(ghInstance, (LPCTSTR)IDD_ANIMATION_BUILDER, hDlg, (DLGPROC)AnimationBuilderDlgProc );
				break;
			case ID_TOOLS_ROTATEFREE:
				DialogBox(ghInstance, (LPCTSTR)IDD_ROTATE_DIALOG, NULL, (DLGPROC)RotateDlgProc );
				break;
			case IDM_MOVE:
				MoveDialogActivate();
				break;
			case ID_SCALE_SCALEDIALOG:
				ScaleDialogActivate();
				break;
			case ID_SCALE_SCALETO15GRID:
				ModelConvScaleTo15mGrid( m_MainSceneObject.GetModelHandle() );
				break;
			case ID_SCALE_SCALEUPOLDTUMODEL:
				ModelConvScaleModelFree( m_MainSceneObject.GetModelHandle(), 2500.0f, 2500.0f, 2500.0f );
				break;
			case ID_FILE_BATCHPROCESS_SCALEMODELS:
				DialogBox(ghInstance, (LPCTSTR)IDD_BATCH_RESCALE, NULL, (DLGPROC)BatchRescaleDlgProc );
				break;
			case ID_FILE_BATCHPROCESS_CONVERTMODELS:
				DialogBox(ghInstance, (LPCTSTR)IDD_BATCH_CONVERT, NULL, (DLGPROC)BatchConvertDlgProc );
				break;
			case ID_TOOLS_STICKTOFLOOR:
				ModelConvStickToFloor( m_MainSceneObject.GetModelHandle() );
				break;
			case ID_UVGENERATE_BOX:
				UVGenerate( m_MainSceneObject.GetModelHandle(), 0 );
				break;
			case ID_TOOLS_ROTATE:
				ModelConvRotate90( m_MainSceneObject.GetModelHandle() );
				break;
			case ID_SELECTION_SELECTALLWITHSAMEMATERIAL:
				ModelConvSelectAllWithSameMaterial( m_MainSceneObject.GetModelHandle() );
				break;
			case IDC_REFRESH_TEXTURE:
				ModelConvReloadTexture();
				break;
			case ID_HELP_HELPMANUAL:
				ShellExecute( GetDesktopWindow(), "Open", "ModelConverter.chm",0, 0, SW_SHOWNORMAL );
				break;
			case IDC_DELETE_FACES:
				ModelConvDeleteFaces();
				break;
			case IDC_AXIS_SNAP:
				PlaneCutterAxisSnap();
				break;
			case IDC_RESET_PLANE:
				PlaneCutterResetPlane();
				break;
			case ID_SELECTION_INVERTSELECTION:
				ModelConvInvertSelection();
				break;
			case IDC_UVUNWRAP:
				ModelConvStartUVUnwrap();
				break;
			case IDC_SUBDIVIDE:
				ModelConvSubdivideSelectedFaces();
				break;
			case IDC_SELECT_CONNECTED:
				{
				int		nSelectedFaceNum = m_MainSceneObject.GetSingleSelectedFace();

					if ( nSelectedFaceNum != NOTFOUND )
					{
						ModelConvSelectAllConnectedFaces( nSelectedFaceNum );
					}
				}
				break;
			case IDC_ASSIGN_MATERIAL:
				DialogBox(ghInstance, (LPCTSTR)IDD_MATERIAL_ASSIGN, mhwndMainDialog, (DLGPROC)ModelConverterAssignMaterialDlgProc );
				break;
			case ID_HELP_PANIC:
				DialogBox(ghInstance, (LPCTSTR)IDD_DIALOG1, NULL, (DLGPROC)PanicDlgProc );
				break;				
			case ID_RENDERING_CLEAROVERRIDETEXTURE:
				ModelConvClearTexture();
				break;
			case IDM_SELECT_TEXTURE:
				ModelConvSelectTexture();
				break;
			case IDC_CONTROL_POLYSLICE:
				ModelConvSetPickerMode(3);
				break;
			case IDC_CONTROL_VERTEXPICKER:
				ModelConvSetPickerMode(2);
				break;
			case IDC_CONTROL_PICKER:
				ModelConvSetPickerMode(1);
				break;
			case IDC_CONTROL_CAMERA:
				ModelConvSetPickerMode(0);
				break;
			case ID_TOOLS_SCALETOTILESIZEX:
				ModelConvScaleToTileSize(0);
				break;
			case ID_TOOLS_CENTREHORIZONTALLY:
				ModelConvCentreHorizontally();
				break;
			case ID_TOOLS_SCALETOTILESIZEY:
				ModelConvScaleToTileSize(1);
				break;
			case ID_TOOLS_PRELIGHT:
				DialogBox(ghInstance, (LPCTSTR)IDD_PRELIGHT_OPTIONS, NULL, (DLGPROC)PrelightDlgProc );			
				break;
			case ID_TOOLS_RESETVERTEXCOLOURS:
				ModelConvResetVertexColours( m_MainSceneObject.GetModelHandle() );
				break;
			case ID_TOOLS_RECALCNORMALS_ALLNORMALSUP:
				ModelConvSetNormals( m_MainSceneObject.GetModelHandle(), ALL_NORMALS_UP );
				ModelConverterDisplayFrame( TRUE );
				break;
			case ID_RECALCNORMALS_STANDARDMODE2:
				ModelConvFixNormals( m_MainSceneObject.GetModelHandle(), SMOOTHED_NORMALS );
				ModelConverterDisplayFrame( TRUE );
				break;
			case ID_TOOLS_RECALCNORMALS_STANDARDFACEORIENTATION:
				ModelConvFixNormals( m_MainSceneObject.GetModelHandle(), FLAT_FACES );
				ModelConverterDisplayFrame( TRUE );
				break;
			case ID_TOOLS_REVERSENORMALS:
				ModelConvReverseNormals( m_MainSceneObject.GetModelHandle() );
				ModelConverterDisplayFrame( TRUE );
				break;
			case ID_TOOLS_TURNINSIDEOUT:

				if ( m_MainSceneObject.GetNumFacesSelected() > 0 )
				{
					m_MainSceneObject.FixInsideOutSelectedFaces();
				}
				else
				{
					ModelConvFixInsideOutModel( m_MainSceneObject.GetModelHandle() );
				}
				ModelConverterDisplayFrame( TRUE );
				break;
			case IDM_FLIP_FACES:
				ModelConvReverseFaceOrientation( m_MainSceneObject.GetModelHandle() );
				break;
			case ID_TOOLS_FIXBUILDINGSCALE:
				ModelConvFixBuildingScale( m_MainSceneObject.GetModelHandle() );
				break;
			case ID_RENDERING_MENUOPTIONS_SCALEMODE_101M:
				ModelConvSetScaleUnitsOption(1, TRUE);
				break;
			case ID_RENDERING_MENUOPTIONS_SCALEMODE_10100FTTHEUNIVERSALMODE:
				ModelConvSetScaleUnitsOption(0, TRUE );
				break;
			case IDC_SELECT_VERTS:
				ModelConvSetVertexSelectionMode( 0 );
				break;
			case IDC_MOVE_VERTS:
				ModelConvSetVertexSelectionMode( 1 );
				break;
			case IDC_ROTATE_VERTS:
				ModelConvSetVertexSelectionMode( 2 );
				break;
			case IDC_SCALE_VERTS:
				ModelConvSetVertexSelectionMode( 3 );
				break;
			case ID_ANIMATIONS_SETVERTICALTURRETATTACHPOINT:
				if ( mnPickerControlMode == CONTROL_VERTEXPICKER )
				{
					TurretAttachDlgInit( 1, m_MainSceneObject.GetFirstSelectedVertex() );
				}
				else
				{
					TurretAttachDlgInit( 1, NOTFOUND );
				}
				break;
			case ID_ANIMATIONS_SETHORIZONTALTURRETATTACHPOINT:
				if ( mnPickerControlMode == CONTROL_VERTEXPICKER )
				{
					TurretAttachDlgInit( 0, m_MainSceneObject.GetFirstSelectedVertex() );
				}
				else
				{
					TurretAttachDlgInit( 0, NOTFOUND );
				}
				break;
			case ID_SPECIAL_SETEFFECTATTACHPOINT:
				if ( mnPickerControlMode == CONTROL_VERTEXPICKER )
				{
					TurretAttachDlgInit( 7, m_MainSceneObject.GetFirstSelectedVertex() );
				}
				else
				{
					TurretAttachDlgInit( 7, NOTFOUND );
				}
				break;
			case ID_SPECIAL_SETGUNFIREATTACHVERTEX:
				if ( mnPickerControlMode == CONTROL_VERTEXPICKER )
				{
					TurretAttachDlgInit( 2, m_MainSceneObject.GetFirstSelectedVertex() );
				}
				else
				{
					TurretAttachDlgInit( 2, NOTFOUND );
				}
				break;
			case ID_ANIMATIONS_SETWHEELATTACHPOINT1:
				if ( mnPickerControlMode == CONTROL_VERTEXPICKER )
				{
					TurretAttachDlgInit( 3, m_MainSceneObject.GetFirstSelectedVertex() );
				}
				else
				{
					TurretAttachDlgInit( 3, NOTFOUND );
				}
				break;
			case ID_ANIMATIONS_SETWHEELATTACHPOINT2:
				if ( mnPickerControlMode == CONTROL_VERTEXPICKER )
				{
					TurretAttachDlgInit( 4, m_MainSceneObject.GetFirstSelectedVertex() );
				}
				else
				{
					TurretAttachDlgInit( 4, NOTFOUND );
				}
				break;
			case ID_ANIMATIONS_SETWHEELATTACHPOINT3:
				if ( mnPickerControlMode == CONTROL_VERTEXPICKER )
				{
					TurretAttachDlgInit( 5, m_MainSceneObject.GetFirstSelectedVertex() );
				}
				else
				{
					TurretAttachDlgInit( 5, NOTFOUND );
				}
				break;
			case ID_ANIMATIONS_SETWHEELATTACHPOINT4:
				if ( mnPickerControlMode == CONTROL_VERTEXPICKER )
				{
					TurretAttachDlgInit( 6, m_MainSceneObject.GetFirstSelectedVertex() );
				}
				else
				{
					TurretAttachDlgInit( 6, NOTFOUND );
				}
				break;
			case ID_SUBMODELS_VIEWEDITSUBMODELS:
				SubModelBrowserInit( hDlg );
				break;
			case IDM_FILE_EXIT:
				EndDialog(hDlg, 0);
				break;
			case ID_SUBMODELS_LEVELOFDETAIL_EXTRACTLODS:
				ModelConvExtractLODs();
				break;
			case IDM_LOD_SETTINGS:
				DialogBox(ghInstance, (LPCTSTR)IDD_LOD_SETTINGS, NULL, (DLGPROC)LODSettingsDlgProc );
				break;
			case ID_ANIMATIONS_ADDHORIZTURRET:
				ModelConvOpenDialog( kHORIZONTAL_TURRET );
				break;
			case ID_ANIMATIONS_ADDVERTICALTURRET:
				ModelConvOpenDialog( kVERTICAL_TURRET );
				break;
			case ID_ANIMATIONS_ADDWHEELMODEL1:
				ModelConvOpenDialog( kWHEEL1 );
				break;
			case ID_ANIMATIONS_ADDWHEELMODEL2:
				ModelConvOpenDialog( kWHEEL2 );
				break;
			case ID_LEVELOFDETAIL_ADDMEDLODMODEL:
				ModelConvOpenDialog( kMEDLOD );
				break;
			case ID_TOOLS_CONVERTYUPTOZUP:
				ModelConvConvertYUpToZUp( m_MainSceneObject.GetModelHandle() );
				ModelConverterDisplayFrame( TRUE );
				break;
			case ID_LEVELOFDETAIL_ADDLOWLODMODEL:
				ModelConvOpenDialog( kLOWLOD );
				break;
			case IDM_ADD_COLLISION_MODEL:
				ModelConvOpenDialog( kCOLLISION );
				break;
			case IDM_FILE_SAVE:
				ModelConvSave();
				break;
			case ID_FILE_SAVEAS:
				ModelConvSaveAsDialog(m_MainSceneObject.GetModelHandle(), TRUE );
				break;
			case IDM_FILE_NEW:
				m_MainSceneObject.Reset();
				break;
			case IDM_FILE_OPEN:
				ModelConvOpenDialog( kMAIN );
				break;
			case ID_ANIMATIONS_BLENDING_LINEAR:
				if ( (GetMenuState( GetMenu(mhwndMainDialog), ID_ANIMATIONS_BLENDING_LINEAR, 0 ) & MF_CHECKED) != 0 )
				{
					CheckMenuItem(GetMenu(mhwndMainDialog), ID_ANIMATIONS_BLENDING_LINEAR, MF_UNCHECKED);
					
				}
				else
				{
					CheckMenuItem(GetMenu(mhwndMainDialog), ID_ANIMATIONS_BLENDING_LINEAR, MF_CHECKED);
					
				}
				ModelConverterDisplayFrame( FALSE );
				break;
			case ID_EXPORT_THUMBNAIL:
				ModelConvExportThumbnail();
				break;

			case IDM_DRAW_GRID_LINES:
				if ( (GetMenuState( GetMenu(mhwndMainDialog), IDM_DRAW_GRID_LINES, 0 ) & MF_CHECKED) != 0 )
				{
					CheckMenuItem(GetMenu(mhwndMainDialog), IDM_DRAW_GRID_LINES, MF_UNCHECKED);
					mboDrawGridLines = FALSE;
				}
				else
				{
					CheckMenuItem(GetMenu(mhwndMainDialog), IDM_DRAW_GRID_LINES, MF_CHECKED);
					mboDrawGridLines = TRUE;
				}
				ModelConverterDisplayFrame( FALSE );
				break;
			case ID_RENDERING_SHOWNORMALS:
				if ( (GetMenuState( GetMenu(mhwndMainDialog), ID_RENDERING_SHOWNORMALS, 0 ) & MF_CHECKED) != 0 )
				{
					CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_SHOWNORMALS, MF_UNCHECKED);
					mboShowNormals = FALSE;
				}
				else
				{
					CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_SHOWNORMALS, MF_CHECKED);
					mboShowNormals = TRUE;
				}
			
				break;
			case ID_RENDERING_LIGHTING_EDITORSTANDARD:
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_EDITORSTANDARD, MF_CHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_SPACE, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_SUNSET, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_LIGHTING_WHITEABOVE, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_LIGHTING_WHITEBELOW, MF_UNCHECKED);
				mnLightingMode = 0;
				ModelConverterAddStandardLighting();
				break;
			case IDC_RESET_CAM_TO_SCENE:
				ModelConvResetCamera();
				ModelConverterSetupCamera();
				ModelConverterDisplayFrame( FALSE );
				break;
			case IDC_RESET_CAM_TO_MODEL:
				ModelConvResetCameraToModel();
				ModelConverterSetupCamera();
				ModelConverterDisplayFrame( FALSE );
				break;
			case IDC_LOCKX:
				ModelConvToggleLock( 0 );
				break;
			case IDC_LOCKY:
				ModelConvToggleLock( 1 );
				break;
			case IDC_LOCKZ:
				ModelConvToggleLock( 2 );
				break;
			case ID_RENDERING_LIGHTING_SPACE:
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_EDITORSTANDARD, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_SPACE, MF_CHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_SUNSET, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_LIGHTING_WHITEABOVE, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_LIGHTING_WHITEBELOW, MF_UNCHECKED);
				mnLightingMode = 1;
				ModelConverterAddStandardLighting();
				break;
			case ID_RENDERING_LIGHTING_SUNSET:
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_EDITORSTANDARD, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_SPACE, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_SUNSET, MF_CHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_LIGHTING_WHITEABOVE, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_LIGHTING_WHITEBELOW, MF_UNCHECKED);
				mnLightingMode = 2;
				ModelConverterAddStandardLighting();
				break;
			case ID_LIGHTING_WHITEABOVE:
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_EDITORSTANDARD, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_SPACE, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_SUNSET, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_LIGHTING_WHITEABOVE, MF_CHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_LIGHTING_WHITEBELOW, MF_UNCHECKED);
				mnLightingMode = 3;
				ModelConverterAddStandardLighting();
				break;
			case ID_IMPORTUVS:
				ModelConverterImportUVs();
				break;
			case ID_LIGHTING_WHITEBELOW:
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_EDITORSTANDARD, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_SPACE, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_LIGHTING_SUNSET, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_LIGHTING_WHITEABOVE, MF_UNCHECKED);
				CheckMenuItem(GetMenu(mhwndMainDialog), ID_LIGHTING_WHITEBELOW, MF_CHECKED);
				mnLightingMode = 4;
				ModelConverterAddStandardLighting();
				break;
			case ID_SHADOWS_NONE:
				ModelConvSetShadowMode(0);
				break;
			case ID_SHADOWS_SHADOW_MAP:
				ModelConvSetShadowMode(1);
				break;
			case ID_SHADOWS_SHADOW_VOLUME:
				ModelConvSetShadowMode(2);
				break;
			case ID_FILE_SAVESELECTIONAS:
				ModelConvSaveSelection();
				break;
			case ID_TOOLS_DELETEISOLATEDVERTICES:
				// TODO - Remove all vertices not referenced by a face
				ModelConvDeleteIsolatedVertices( m_MainSceneObject.GetModelHandle() );
				break;
			case ID_TOOLS_SEPARATEVERTS:
				ModelConvSeparateVerts( m_MainSceneObject.GetModelHandle() );
				break;
			case ID_RENDERING_MODELDISPLAY_NORMAL:
				ModelConvSetModelRenderingOption( 0 );
				break;
			case ID_RENDERING_MODELDISPLAY_POLYEDGES:
				ModelConvSetModelRenderingOption( 1 );
				break;
			case ID_RENDERING_MODELDISPLAY_SHOWVERTICES:
				ModelConvSetModelRenderingOption( 2 );
				break;
			case ID_SCENETOOLS_CREATESCENEFROMMODEL:
				DialogBox(ghInstance, (LPCTSTR)IDD_MODEL_TO_SCENE_DIALOG, NULL, (DLGPROC)ModelToSceneDlg );		
				break;
			case ID_SCENETOOLS_ADDPRIMITIVE_WALLCIRCLE:
				AddCurvedWall();
				ModelConverterDisplayFrame( TRUE );
				break;
			case ID_ADDPRIMITIVE_CUBOID:
				AddPrimitiveCuboid();
				ModelConverterDisplayFrame( TRUE );
				break;
			case ID_LEVELOFDETAIL_DISPLAY_DEPENDONVIEW:
				ModelConvSetLODDisplayOption( 0 );
				break;
			case ID_SUBMODELS_WHEELMODE_NORMAL:
				ModelConvSetWheelModeOption(0);
				break;
			case ID_SUBMODELS_WHEELMODE_PLANETYPE1:
				ModelConvSetWheelModeOption(1);
				break;
			case ID_SUBMODELS_WHEELMODE_PLANETYPE2:
				ModelConvSetWheelModeOption(2);
				break;
			case ID_SUBMODELS_WHEELMODE_HELICOPTER:
				ModelConvSetWheelModeOption(3);
				break;
			case ID_SUBMODELS_WHEELMODE_HELICOPTER4:
				ModelConvSetWheelModeOption(4);
				break;
			case ID_SUBMODELS_WHEELMODE_BIKE:
				ModelConvSetWheelModeOption(5);
				break;
			case ID_SUBMODELS_WHEELMODE_WHEELS_YFORWARD:
				ModelConvSetWheelModeOption(6);
				break;
			case ID_SUBMODELS_WHEELMODE_BIKE_YFORWARD:
				ModelConvSetWheelModeOption(7);
				break;
			case ID_SUBMODELS_WHEELMODE_PLANETYPE1_YFORWARD:
				ModelConvSetWheelModeOption(8);
				break;
			case ID_SUBMODELS_WHEELMODE_PLANETYPE2_YFORWARD:
				ModelConvSetWheelModeOption(9);
				break;
			case ID_LEVELOFDETAIL_DISPLAY_HIGH:
				ModelConvSetLODDisplayOption( 1 );
				break;
			case ID_LEVELOFDETAIL_DISPLAY_MEDLODALWAYS:
				ModelConvSetLODDisplayOption( 2 );
				break;
			case ID_LEVELOFDETAIL_DISPLAY_LOWLODALWAYS:
				ModelConvSetLODDisplayOption( 3 );
				break;
			case ID_RENDERING_BACKGROUNDCOLOUR_GREY:
				ModelConvSetBackgroundColourOption( 0 );
				break;
			case ID_RENDERING_BACKGROUNDCOLOUR_GREEN:
				ModelConvSetBackgroundColourOption( 1 );
				break;
			case ID_RENDERING_BACKGROUNDCOLOUR_BLACK:
				ModelConvSetBackgroundColourOption( 2 );
				break;
			case ID_RENDERING_BACKGROUNDCOLOUR_WHITE:
				ModelConvSetBackgroundColourOption( 3 );
				break;
			case ID_RENDERING_BACKGROUNDCOLOUR_BLUE:
				ModelConvSetBackgroundColourOption( 4 );
				break;
			case ID_RENDERING_BACKGROUNDCOLOUR_RED:
				ModelConvSetBackgroundColourOption( 5 );
				break;
			case ID_RENDERING_GOURAUDSHADING:
				if ( (GetMenuState( GetMenu(mhwndMainDialog), ID_RENDERING_GOURAUDSHADING, 0 ) & MF_CHECKED) != 0 )
				{
					CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_GOURAUDSHADING, MF_UNCHECKED);
					g_pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
				}
				else
				{
					CheckMenuItem(GetMenu(mhwndMainDialog), ID_RENDERING_GOURAUDSHADING, MF_CHECKED);
					g_pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
				}
				ModelConverterDisplayFrame( FALSE );
				break;
			}
			break;
		}
		break;
	case WM_SIZE:
		mboMinimised = FALSE;
		switch ( wParam )
		{
		case SIZE_MAXIMIZED:
			ModelConvResizeWindow( LOWORD(lParam), HIWORD(lParam) );
			ModelConvSaveExportFlags();
			break;
		case SIZE_RESTORED:
			ModelConvResizeWindow( LOWORD(lParam), HIWORD(lParam) );
			ModelConvSaveExportFlags();
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



/***************************************************************************
 * Function    : WinMain
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
//INITCOMMONCONTROLSEX	xCommStruct;
FILE*	pFile;
char	acString[256];
char*	pcRunner;

	ghInstance = hInstance;
#ifdef SECURITY_FEATURES
	LoadUniqueID();
#endif

	GetModuleFileName( NULL, macModelConvRootFolder, 256 );
	pcRunner = macModelConvRootFolder + strlen( macModelConvRootFolder );
	while( ( *pcRunner != '\\' ) &&
		   ( *pcRunner != '/' ) &&
		   ( pcRunner > macModelConvRootFolder ) )
	{
		pcRunner--;
	}
	if ( ( *pcRunner == '\\' ) ||
		 ( *pcRunner != '/' ) )
	{
		pcRunner[1] = 0;
	}

	SetCurrentDirectory( macModelConvRootFolder );
//	GetCurrentDirectory( 256, macModelConvRootFolder );

	sprintf( acString, "%s/modelconvdirs.opt", macModelConvRootFolder );
	pFile = fopen( acString, "rb" );
	if ( pFile == NULL )
	{
		strcpy( macModelConvLastSaveFolder, macModelConvRootFolder );
		strcpy( macModelConvLastLoadFolder, macModelConvRootFolder );
	}
	else
	{
		fread( macModelConvLastSaveFolder, 256,1,pFile );
		fread( macModelConvLastLoadFolder, 256,1,pFile );
		fclose( pFile );
	}
	mhbmpMoveSel = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP4)); 
	mhbmpRotSel = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP5)); 	
	mhbmpZoomSel = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP6)); 
	mhbmpMove = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP3)); 
	mhbmpRot = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP2)); 	
	mhbmpZoom = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_BITMAP1)); 

#ifdef _DEBUG
	SysRegisterDebugPrintHandler( ModelConvPrintToConsole );
#endif
	m_MainSceneObject.SetViewInterface( &m_sModelConvViewInterface );
	if ( lpCmdLine[0] != 0 )
	{
		if ( lpCmdLine[0] == '\"' )
		{
		int		nStrlen;
			strcpy( mszCommandLineFile, lpCmdLine + 1 );
			nStrlen = strlen( mszCommandLineFile );
			if ( mszCommandLineFile[nStrlen-1] == '\"' )
			{
				mszCommandLineFile[nStrlen-1] = 0;
			}
		}
		else
		{
			strcpy( mszCommandLineFile, lpCmdLine );
		}
	}
	else
	{
		mszCommandLineFile[0] = 0;
	}

//	xCommStruct.dwSize = sizeof( xCommStruct );
//	xCommStruct.dwICC = ICC_LISTVIEW_CLASSES;

//	InitCommonControlsEx( &xCommStruct );
	GetCurrentDirectory( 256, macBatchConvFolder );

	DialogBox(hInstance, (LPCTSTR)IDD_MODEL_CONVERTER_MAIN, NULL, (DLGPROC)ModelConverterMainDlgProc );

	return 0;
}


int DevLog( int eLogType, const char *format, ... )
{
	if ( 1 )
	{
	char		acString[512];
	char		acMessage[512];
	va_list		marker;
	uint32*		pArgs;
	int			nLen;
	BOOL		bEndsWithNewLine = FALSE;

		pArgs = (uint32*)( &format ) + 1;

		va_start( marker, format );     
		vsprintf( acMessage, format, marker );

		nLen = strlen(acMessage);
		if ( nLen > 0)
		{
		uint32		ulTick = SysGetTick();

			if ( acMessage[nLen-1] == '\n')
			{
				bEndsWithNewLine = TRUE;
			}
			sprintf( acString, "[DEV-%d] %03d.%03d ", eLogType, (ulTick / 1000)%1000, ulTick % 1000 );
			strcat( acString, acMessage );
			OutputDebugString( acString );
			if ( bEndsWithNewLine == FALSE )
			{
				OutputDebugString( "\r\n" );
			}
			return( 1 );
		}
	}
	return( 0 );
}
