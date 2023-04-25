#include <stdio.h>
#include <windows.h>
#include <CommCtrl.h>
#include <shlobj.h>
#include "resource.h"

#include <d3dx9.h>

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#include "../LibCode/Engine/ModelFiles.h"
#include "../LibCode/Engine/ModelRendering.h"
#include "../LibCode/Engine/DirectX/ModelRenderingDX.h"

#include "Animation Builder.h"
#include "ModelConverter.h"

#define		MAX_KEYFRAME_MODELS		256

int		mnModelsInAnimation = 0;
int		manKeyframeModelHandles[ MAX_KEYFRAME_MODELS ];
int		mnSelectedKeyFrameNum = NOTFOUND;

HWND	mhwndAnimationBuilderDialog = NULL;

int		mnAnimationHandle = NOTFOUND;

char*	maszAnimationUses[MAX_ANIMATION_USES] = 
{
	"<None>",
	"Walk",
	"Run",
	"Die",
	"Jump",
	"Standing",
	"Enter Building",
	"Generic Weapon Fire",
	"Fire Plasma",
	"Fire Machinegun",
	"Fire Special 1",
	"Move Stop",
	"Move Start",
	"Wave",
	"Smoking",
	"Standing bored",
	"Strafe",
	"Jetpack",
	"Generic Reload",
	"Eat",
	"Drink",
	"Duck",
	"Duck-stand up",
	"Crawl",
	"Lay Prone",
	"Dive Left",
	"Dive Right",
	"Forward Roll",
	"Backward Roll",
	"Kick",
	"Gesture 1",
	"Gesture 2",
	"Gesture 3",
	"Dead",
	"Sat down",
	"Collision",
	"Fire Weapon Model 1",
	"Fire Weapon Model 2",
	"Fire Weapon Model 3",
	"Fire Weapon Model 4",
	"Fire Weapon Model 5",
	"Fire Weapon Model 6",
	"Fire Weapon Model 7",
	"Fire Weapon Model 8",

	"Walk Damaged",
	"Run Damaged",
	"Generic Fire - Walking",
	"Generic Fire - Running",
	"Generic Fire - Crawling",
	"Generic Fire - Prone",

	"Pose 1- BlendIn",
	"Pose 1- Base",
	"Pose 1- Idle1",
	"Pose 1- Idle2",
	"Pose 1- Idle3",
	"Pose 1- Idle4",
	"Pose 1- Action1",
	"Pose 1- Action2",
	"Pose 1- Action3",
	"Pose 1- Action4",
	"Pose 1- BlendOut",

	"Pose 2- BlendIn",
	"Pose 2- Base",
	"Pose 2- Idle1",
	"Pose 2- Idle2",
	"Pose 2- Idle3",
	"Pose 2- Idle4",
	"Pose 2- Action1",
	"Pose 2- Action2",
	"Pose 2- Action3",
	"Pose 2- Action4",
	"Pose 2- BlendOut",

	"Pose 3- BlendIn",
	"Pose 3- Base",
	"Pose 3- Idle1",
	"Pose 3- Idle2",
	"Pose 3- Idle3",
	"Pose 3- Idle4",
	"Pose 3- Action1",
	"Pose 3- Action2",
	"Pose 3- Action3",
	"Pose 3- Action4",
	"Pose 3- BlendOut",

	"Pose 4- BlendIn",
	"Pose 4- Base",
	"Pose 4- Idle1",
	"Pose 4- Idle2",
	"Pose 4- Idle3",
	"Pose 4- Idle4",
	"Pose 4- Action1",
	"Pose 4- Action2",
	"Pose 4- Action3",
	"Pose 4- Action4",
	"Pose 4- BlendOut",

	"Random Idle 1",
	"Random Idle 2",
	"Random Idle 3",
	"Random Idle 4",
	"Random Idle 5",
	"Random Idle 6",
	"Random Idle 7",
	"Random Idle 8",
	"Random Idle 9",
	"Random Idle 10",
	"Random Idle 11",
	"Random Idle 12",
	"Random Idle 13",
	"Random Idle 14",
	"Random Idle 15",
	"Random Idle 16",
};


MODEL_KEYFRAME_DATA		maxBuilderKeyframeData[MAX_KEYFRAMES_IN_MODEL];


void AnimationBuilderEditKeyframe( void )
{
char	acString[256];

	sprintf( acString, "%d", maxBuilderKeyframeData[ mnSelectedKeyFrameNum ].uwKeyframeTime );
	SetDlgItemText( mhwndAnimationBuilderDialog, IDC_KEYFRAME_TIME_EDIT, acString );

	EnableWindow( GetDlgItem( mhwndAnimationBuilderDialog, IDC_KEYFRAME1 ), TRUE );
	EnableWindow( GetDlgItem( mhwndAnimationBuilderDialog, IDC_KEYSTRING1 ), TRUE );
	EnableWindow( GetDlgItem( mhwndAnimationBuilderDialog, IDC_KEYFRAME_TIME_EDIT ), TRUE );
	EnableWindow( GetDlgItem( mhwndAnimationBuilderDialog, IDC_KEYSTRING3 ), TRUE );
	EnableWindow( GetDlgItem( mhwndAnimationBuilderDialog, IDC_REMOVE_KEYFRAME), TRUE );
	sprintf( acString, "Properties for Keyframe %d", mnSelectedKeyFrameNum );
	SetDlgItemText( mhwndAnimationBuilderDialog,IDC_KEYFRAME1, acString ); 

	EnableWindow( GetDlgItem( mhwndAnimationBuilderDialog, IDC_CHECK1), TRUE );

	EnableWindow( GetDlgItem( mhwndAnimationBuilderDialog, IDC_ANIMATION_USE_LIST ), TRUE );
	SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_ANIMATION_USE_LIST, CB_SETCURSEL, maxBuilderKeyframeData[ mnSelectedKeyFrameNum ].bAnimationUse, 0 );

	if ( maxBuilderKeyframeData[mnSelectedKeyFrameNum ].bAnimationTriggerCode != 0 )
	{
		SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_CHECK1,BM_SETCHECK, BST_CHECKED, 0 );
		EnableWindow( GetDlgItem( mhwndAnimationBuilderDialog, IDC_EDIT1), TRUE );
		sprintf( acString, "%d", maxBuilderKeyframeData[mnSelectedKeyFrameNum ].bAnimationTriggerCode );
		SetDlgItemText( mhwndAnimationBuilderDialog, IDC_EDIT1, acString );
	}
	else
	{
		SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_CHECK1,BM_SETCHECK, BST_UNCHECKED, 0 );
		EnableWindow( GetDlgItem( mhwndAnimationBuilderDialog, IDC_EDIT1), FALSE );
	}
}


/***************************************************************************
 * Function    : AnimationBuilderAddModelKeyframeDlg
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
BOOL		AnimationBuilderValidateKeyframes( void )
{
int		nLoop;
int		nHandle;
int		nNumFaces;
char	acString[512];

	if ( mnModelsInAnimation < 2 )
	{
		return( FALSE );
	}

	nHandle = manKeyframeModelHandles[ 0 ];
	nNumFaces = ModelGetStats( nHandle )->nNumIndices / 3;

	for( nLoop = 0; nLoop < mnModelsInAnimation; nLoop++ )
	{
		nHandle = manKeyframeModelHandles[ nLoop ];
		if ( nNumFaces != (ModelGetStats( nHandle )->nNumIndices / 3) )
		{
			sprintf( acString, "Model Keyframe number %d does not have the same number of faces as the earlier keyframes.\nTo build an animation, all keyframes MUST have the same number of faces", nLoop );		
			MessageBox( mhwndAnimationBuilderDialog, acString, "Vertex count error in animations", MB_OK |MB_ICONEXCLAMATION );
			return( FALSE );
		}
	}

	return( TRUE );
}


/***************************************************************************
 * Function    : AnimationBuilderCloneKeyframe
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
int		AnimationBuilderCloneKeyframe( MODEL_RENDER_DATA* pxModelData, int nKeyframe )
{
VECT*		pxVerts;
VECT*		pxNormals;
MODEL_RENDER_DATA*		pxNewModelData;
int		nNewHandle;
CUSTOMVERTEX*		pxOrigVerts;
ushort*		puwOrigIndices;
CUSTOMVERTEX*		pxNewVerts;
ushort*		puwNewIndices;
int		nLoop;

	nNewHandle = ModelRenderGetNextHandle();
	pxNewModelData = &maxModelRenderData[ nNewHandle ];

	pxNewModelData->xStats = pxModelData->xStats;
	pxNewModelData->bModelType = ASSETTYPE_STATIC_MESH;
	if ( pxModelData->xStats.acFilename[0] != '[' )
	{
		sprintf( pxNewModelData->xStats.acFilename, "[Key %d] Packed Data", nKeyframe );//, pxModelData->xStats.acFilename );
	}
	else
	{
		sprintf( pxNewModelData->xStats.acFilename, "[Key %d] %s", nKeyframe, &pxModelData->xStats.acFilename[8] );
	}
	
	// Create the mesh here now we know the vertex and face counts..
	EngineCreateMesh( pxModelData->xStats.nNumIndices/3,pxModelData->xStats.nNumVertices, &pxNewModelData->pxBaseMesh, 2 );

	if ( pxNewModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxOrigVerts ) );
		pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwOrigIndices ) );

		pxNewModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxNewVerts ) );
		pxNewModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwNewIndices ) );
		pxVerts = pxModelData->pxVertexKeyframes + nKeyframe; 
		pxNormals = pxModelData->pxNormalKeyframes + nKeyframe;

		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
		{
			pxNewVerts[ nLoop ] = pxOrigVerts[ nLoop ];

			pxNewVerts[nLoop].position.x = pxVerts->x;
			pxNewVerts[nLoop].position.y = pxVerts->y;
			pxNewVerts[nLoop].position.z = pxVerts->z;
			if ( pxModelData->pxNormalKeyframes != NULL )
			{
				pxNewVerts[nLoop].normal.x = pxNormals->x;
				pxNewVerts[nLoop].normal.y = pxNormals->y;
				pxNewVerts[nLoop].normal.z = pxNormals->z;
			}
			pxVerts += pxModelData->xStats.nNumVertKeyframes;
			pxNormals += pxModelData->xStats.nNumVertKeyframes;
		}

		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumIndices; nLoop++ )
		{
			*(puwNewIndices++) = *(puwOrigIndices++);
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
		pxModelData->pxBaseMesh->UnlockIndexBuffer();
		pxNewModelData->pxBaseMesh->UnlockVertexBuffer();
		pxNewModelData->pxBaseMesh->UnlockIndexBuffer();

		return( nNewHandle );
	}

	return( NOTFOUND );
}


/***************************************************************************
 * Function    : AnimationBuilderReBuildBaseMesh
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
void		AnimationBuilderReBuildBaseMesh( MODEL_RENDER_DATA* pxModelData )
{
EngineMesh*		pxNewMesh;
CUSTOMVERTEX*	pxVertices;
int		nVertLoop;
VECT*		pxVerts;
ushort*		puwOrigIndices;
ushort*		puwIndices;
CUSTOMVERTEX*		pxOrigVerts;

	// Create the mesh here now we know the vertex and face counts..
	EngineCreateMesh( pxModelData->xStats.nNumIndices/3, pxModelData->xStats.nNumVertices, &pxNewMesh, 1 );

	if ( pxNewMesh != NULL )
	{
		pxVerts = pxModelData->pxVertexKeyframes;
		pxNewMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

		pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxOrigVerts ) );
		pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwOrigIndices ) );

		for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
		{
			*(pxVertices) = pxOrigVerts[ *(puwOrigIndices++) ];

			pxVertices->position.x = pxVerts->x;
			pxVertices->position.y = pxVerts->y;
			pxVertices->position.z = pxVerts->z;

			pxVertices++;
			pxVerts += pxModelData->xStats.nNumVertKeyframes;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
		pxModelData->pxBaseMesh->UnlockIndexBuffer();
		pxNewMesh->UnlockVertexBuffer();

		// Now set the basemesh indices to 0,1,2  3,4,5 etc which the keyframe animation expects
		pxNewMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
		for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumIndices; nVertLoop++ )
		{
			*(puwIndices++) = (ushort)(nVertLoop);
		}
		pxNewMesh->UnlockIndexBuffer();

		// TODO - If the base mesh had attributes we should copy em to the new mesh

/*
		pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_ReadOnly, &pxMeshAttributes );
		if ( pxMeshAttributes )
		{
			for ( nFaceLoop = 0; nFaceLoop < nNumFaces; nFaceLoop++ )
			{
				*pxATMAttribs = (byte)(*pxMeshAttributes);

				pxMeshAttributes++;
				pxATMAttribs++;
			}
			pxModelData->pxBaseMesh->UnlockAttributeBuffer();
		}
*/

		// Release the old model and replace it with the new one
		pxModelData->pxBaseMesh->Release();
		pxModelData->pxBaseMesh = pxNewMesh;
	}


}

/***************************************************************************
 * Function    : AnimationBuilderBuildKeyframes
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
int		AnimationBuilderBuildKeyframes( void )
{
MODEL_RENDER_DATA*	pxModelData = maxModelRenderData;
CUSTOMVERTEX*	pxVertices;
int		nFaceLoop;
int		nFacesInMesh;
int		nHandle;
int		nLoop;
VECT*	pxVertexBuffer;
VECT*	pxNormalBuffer;
VECT*	pxVertexBase;
VECT*	pxNormalBase;
ushort*		puwIndices;

	nHandle = manKeyframeModelHandles[ 0 ];
	pxModelData = maxModelRenderData + nHandle;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nFacesInMesh = pxModelData->xStats.nNumIndices/3;

		// Allocate mem for the keyframe data for each vertex
		pxVertexBuffer = (VECT*)( malloc( (nFacesInMesh*3) * mnModelsInAnimation * sizeof(VECT) ) );
		pxNormalBuffer = (VECT*)( malloc( (nFacesInMesh*3) * mnModelsInAnimation * sizeof(VECT) ) );
		pxVertexBase = pxVertexBuffer;		
		pxNormalBase = pxNormalBuffer;

		// Read all the vertex coordinates of all the models
		for ( nLoop = 0; nLoop < mnModelsInAnimation; nLoop++ )
		{
			pxVertexBuffer = pxVertexBase + nLoop;
			pxNormalBuffer = pxNormalBase + nLoop;

			nHandle = manKeyframeModelHandles[ nLoop ];
			pxModelData = maxModelRenderData + nHandle;

			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
			pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
			for ( nFaceLoop = 0; nFaceLoop < nFacesInMesh; nFaceLoop++ )
			{
				*(pxVertexBuffer) = *(VECT*)(&pxVertices[ puwIndices[0] ].position);
				VectNormalize( &pxVertices[ puwIndices[0] ].normal );
				*(pxNormalBuffer) = *(VECT*)(&pxVertices[ puwIndices[0] ].normal);
				pxVertexBuffer += mnModelsInAnimation;
				pxNormalBuffer += mnModelsInAnimation;
				VectNormalize( &pxVertices[ puwIndices[1] ].normal );
				*(pxVertexBuffer) = *(VECT*)(&pxVertices[ puwIndices[1] ].position);
				*(pxNormalBuffer) = *(VECT*)(&pxVertices[ puwIndices[1] ].normal);
				pxVertexBuffer += mnModelsInAnimation;
				pxNormalBuffer += mnModelsInAnimation;
				VectNormalize( &pxVertices[ puwIndices[2] ].normal );
				*(pxVertexBuffer) = *(VECT*)(&pxVertices[ puwIndices[2] ].position);
				*(pxNormalBuffer) = *(VECT*)(&pxVertices[ puwIndices[2] ].normal);
				pxVertexBuffer += mnModelsInAnimation;
				pxNormalBuffer += mnModelsInAnimation;

				puwIndices += 3;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
			pxModelData->pxBaseMesh->UnlockIndexBuffer();
		}
	}

	// Now free all the models we used to build up the keyframe data
	for ( nLoop = 1; nLoop < mnModelsInAnimation; nLoop++ )
	{
		nHandle = manKeyframeModelHandles[ nLoop ];
		ModelFree( nHandle );
		manKeyframeModelHandles[ nLoop ] = NOTFOUND;
	}
	
	// Set the vertex keyframe data we've created to the first model
	nHandle = manKeyframeModelHandles[ 0 ];
	pxModelData = maxModelRenderData + nHandle;
	
	pxModelData->pxVertexKeyframes = pxVertexBase;
	pxModelData->pxNormalKeyframes = pxNormalBase;
	pxModelData->xStats.nNumVertKeyframes = mnModelsInAnimation;
	// Update the models vertex count, as theres probably more vertices now..
	pxModelData->xStats.nNumVertices = nFacesInMesh*3;

	// Copy the builder keyframe data into the model data
	for ( nLoop = 0; nLoop < mnModelsInAnimation; nLoop++ )
	{
		pxModelData->axKeyframeData[ nLoop ] = maxBuilderKeyframeData[ nLoop ];
	}

	// Rebuild the base mesh, so that it also has the correct number of vertices
	AnimationBuilderReBuildBaseMesh( pxModelData );

	pxModelData->bModelType = ASSETTYPE_KEYFRAME_ANIMATION;
	mnModelsInAnimation = 0;

	return( nHandle );

}



void	AnimationBuilderFreeAll( void )
{
int		nLoop;
int		nHandle;

	// Now free all the models we used to build up the keyframe data
	for ( nLoop = 0; nLoop < mnModelsInAnimation; nLoop++ )
	{
		nHandle = manKeyframeModelHandles[ nLoop ];
		ModelFree( nHandle );
		manKeyframeModelHandles[ nLoop ] = NOTFOUND;
	}
	mnModelsInAnimation = 0;
}

/***************************************************************************
 * Function    : AnimationBuilderAddModelKeyframeDlg
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
int		AnimationBuilderAddModelKeyframeDlg( void )
{
OPENFILENAME	ofnOpen;
char		acFileName[256];
char		acString[512];
int		nHandle = NOTFOUND;

	ZeroMemory( &ofnOpen, sizeof( ofnOpen ) );
	acFileName[0] = 0;
	ModelConvGetLastLoadFolder( acString );
	ofnOpen.lStructSize       = sizeof(OPENFILENAME);
    ofnOpen.hwndOwner         = mhwndAnimationBuilderDialog;
    ofnOpen.hInstance         = 0;
    ofnOpen.lpstrFilter       = (LPSTR)"All Supported Formats\0*.x;*.atm;*.fbx;*.3ds\0All Files (*.*)\0*.*\0";
    ofnOpen.lpstrCustomFilter = NULL;
    ofnOpen.nMaxCustFilter    = 0;
    ofnOpen.nFilterIndex      = 1;
    ofnOpen.lpstrFile         = (LPSTR)acFileName;
    ofnOpen.nMaxFile          = sizeof(acFileName);
    ofnOpen.lpstrInitialDir   = acString;
    ofnOpen.lpstrTitle        = "Add model file(s) to animation..";
    ofnOpen.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofnOpen.nFileOffset       = 0;
    ofnOpen.nFileExtension    = 0;

	if ( GetOpenFileName( &ofnOpen ) > 0 )
	{
		ModelConvSetLastUsedLoadPath( acFileName );
		nHandle = ModelLoad( acFileName, 0, 1.0f );
	}

	return( nHandle );
}

void AnimationBuilderUpdateList( void )
{
	if ( mhwndAnimationBuilderDialog != NULL )
	{
	int				nLoop;
	LVITEM			xInsert;
	char			acString[256];
	//char*			szName;
	//ulong			ulIP;
	//ushort		uwPort;
	int				nListTop;

		nListTop = SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_GETTOPINDEX, 0, (LPARAM)0 );

		SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_DELETEALLITEMS, 0, 0 );

		for( nLoop = 0; nLoop < mnModelsInAnimation; nLoop++ )
		{
			xInsert.mask = LVIF_TEXT | LVIF_STATE; 
			xInsert.state = 0; 
			xInsert.stateMask = 0; 
			xInsert.iItem = nLoop;
			xInsert.iSubItem = 0; 
			sprintf( acString, "%d", nLoop );
			xInsert.pszText = acString;
			SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_INSERTITEM, 0, (LPARAM)&xInsert );

			if ( maxBuilderKeyframeData[ nLoop ].bAnimationUse > 0 )
			{
				xInsert.iSubItem = 1; 
			
				sprintf( acString, "Start of %s", maszAnimationUses[ maxBuilderKeyframeData[ nLoop ].bAnimationUse ] );
				xInsert.pszText = acString;
				SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_SETITEM, 0, (LPARAM)&xInsert );
			}

			sprintf( acString, "%d", maxBuilderKeyframeData[ nLoop ].uwKeyframeTime );
			xInsert.iSubItem = 2; 
			xInsert.pszText = acString;
			SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_SETITEM, 0, (LPARAM)&xInsert );

			if ( maxBuilderKeyframeData[ nLoop ].bAnimationTriggerCode != 0 )
			{
				sprintf( acString, "Yes (%d)", maxBuilderKeyframeData[ nLoop ].bAnimationTriggerCode );
				xInsert.iSubItem = 3; 
				xInsert.pszText = acString;
				SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_SETITEM, 0, (LPARAM)&xInsert );
			}

			if ( manKeyframeModelHandles[ nLoop ] != NOTFOUND )
			{
				xInsert.iSubItem = 4; 
				xInsert.pszText = ModelGetStats( manKeyframeModelHandles[ nLoop ] )->acFilename;
				SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_SETITEM, 0, (LPARAM)&xInsert );
			}
		}

		SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_SCROLL, 0, (LPARAM)nListTop*14 );
	}
}


void AnimationBuilderRemoveKeyframe( void )
{
int		nLoop;
//MODEL_RENDER_DATA*		pxModelData;

	if ( mnSelectedKeyFrameNum == NOTFOUND ) return;

	ModelFree( 	manKeyframeModelHandles[ mnSelectedKeyFrameNum ] );
	for ( nLoop = mnSelectedKeyFrameNum; nLoop < (mnModelsInAnimation-1); nLoop++ )
	{
		manKeyframeModelHandles[ nLoop ] = manKeyframeModelHandles[ nLoop+1 ];
		maxBuilderKeyframeData[ nLoop ] = maxBuilderKeyframeData[ nLoop + 1 ];
	}
	manKeyframeModelHandles[ nLoop ] = NOTFOUND;

	mnModelsInAnimation--;
	mnSelectedKeyFrameNum = NOTFOUND;
	AnimationBuilderUpdateList();

}

void	AnimationBuilderExistingKeyframes( int nHandle )
{
MODEL_RENDER_DATA*		pxModelData;
int			nLoop;

	pxModelData = maxModelRenderData + nHandle;

	if ( pxModelData->bModelType == ASSETTYPE_KEYFRAME_ANIMATION )
	{
		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertKeyframes; nLoop++ )
		{
			maxBuilderKeyframeData[ mnModelsInAnimation ] = pxModelData->axKeyframeData[ nLoop ];

			// Now generate a new mesh for each keyframe and store the handles..
			manKeyframeModelHandles[ mnModelsInAnimation ] = AnimationBuilderCloneKeyframe( pxModelData, nLoop );
			mnModelsInAnimation++;
		}
	}
	AnimationBuilderUpdateList();
}


void	AnimationBuilderAddNewKeyframe( int nHandle )
{
int		nKeyframesInNewAnimation;

	nKeyframesInNewAnimation = ModelGetStats( nHandle )->nNumVertKeyframes;
	// If the new model is just a single static mesh
	if ( nKeyframesInNewAnimation == 0 )
	{
		// Just add the model to the list directly
		manKeyframeModelHandles[ mnModelsInAnimation ] = nHandle;

		if ( mnModelsInAnimation == 0 )
		{
			maxBuilderKeyframeData[ mnModelsInAnimation ].bAnimationUse = ANIM_WALK;
			maxBuilderKeyframeData[ mnModelsInAnimation ].uwKeyframeTime = 300;
		}
		else
		{
			maxBuilderKeyframeData[ mnModelsInAnimation ].uwKeyframeTime = 300;
			maxBuilderKeyframeData[ mnModelsInAnimation ].bAnimationUse = 0;
		}

		mnModelsInAnimation++;
	}
	else
	{
		AnimationBuilderExistingKeyframes( nHandle );
	}

	AnimationBuilderUpdateList();

}




/***************************************************************************
 * Function    : AnimationBuilderAddModelKeyframeDlg
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
BOOL		AnimationBuilderAddSequenceDlg( char* pcString )
{
OPENFILENAME	ofnOpen;
char		acFileName[256];
char		acString[512];
int		nHandle = NOTFOUND;

	ZeroMemory( &ofnOpen, sizeof( ofnOpen ) );
	acFileName[0] = 0;
	ModelConvGetLastLoadFolder( acString );

	ofnOpen.lStructSize       = sizeof(OPENFILENAME);
    ofnOpen.hwndOwner         = mhwndAnimationBuilderDialog;
    ofnOpen.hInstance         = 0;
    ofnOpen.lpstrFilter       = (LPSTR)"All Supported Formats\0*.x;*.atm;*.fbx;*.3ds\0All Files (*.*)\0*.*\0";
    ofnOpen.lpstrCustomFilter = NULL;
    ofnOpen.nMaxCustFilter    = 0;
    ofnOpen.nFilterIndex      = 1;
    ofnOpen.lpstrFile         = (LPSTR)acFileName;
    ofnOpen.nMaxFile          = sizeof(acFileName);
    ofnOpen.lpstrInitialDir   = acString;
    ofnOpen.lpstrTitle        = "Choose sequence start model..";
    ofnOpen.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofnOpen.nFileOffset       = 0;
    ofnOpen.nFileExtension    = 0;

	if ( GetOpenFileName( &ofnOpen ) > 0 )
	{
		ModelConvSetLastUsedLoadPath( acFileName );
		sprintf( pcString, acFileName );
		return( TRUE );
	}

	return( FALSE );

}


void  AnimationBuilderAddModelSequence( void )
{
int		nHandle;
char	acString[256];
char*	pcString;
char	acNumBit[64];
int		nVal;

	if ( AnimationBuilderAddSequenceDlg( acString ) == TRUE )
	{
		nVal = strlen( acString );
		while ( ( acString[nVal] != '.' ) &&
				( nVal > 0 ) )
		{
			nVal--;
		}
		if ( nVal != 0 )
		{
			nVal -= 2;
		}
		pcString = &acString[ nVal ];
		acNumBit[0] = pcString[0];
		acNumBit[1] = pcString[1];
		acNumBit[2] = 0;
		nVal = strtol( acNumBit, NULL, 10 );

		nHandle = ModelLoad( acString, 0, 1.0f );
		while ( nHandle != NOTFOUND )
		{
			AnimationBuilderAddNewKeyframe( nHandle );
			
			nVal++;
			sprintf( acNumBit, "%02d", nVal );
			pcString[0] = acNumBit[0];
			pcString[1] = acNumBit[1];
			nHandle = ModelLoad( acString, 0, 1.0f );
		}
	}
}

/***************************************************************************
 * Function    : AnimationBuilderDlgProc
 * Params      :
 * Returns     :
 * Description :
 ***************************************************************************/
void	AnimationBuilderInitLists( void )
{
LVCOLUMN	xColumn;
int			nLoop;
//char		acString[256];

	ZeroMemory( maxBuilderKeyframeData, MAX_KEYFRAMES_IN_MODEL * sizeof( MODEL_KEYFRAME_DATA ) );

	xColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
	xColumn.fmt = LVCFMT_RIGHT;
	xColumn.pszText = "Keyframe";
	xColumn.cx = 50;
	SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_INSERTCOLUMN, 0, (LPARAM)&xColumn );

	xColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM;
	xColumn.fmt = LVCFMT_RIGHT;
	xColumn.pszText = "Start of Anim";
	xColumn.cx = 130;
	xColumn.iSubItem = 1;
	SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_INSERTCOLUMN, 1 , (LPARAM)&xColumn );

	xColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM;
	xColumn.fmt = LVCFMT_RIGHT;
	xColumn.pszText = "Time";
	xColumn.cx = 45;
	xColumn.iSubItem = 2;
	SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_INSERTCOLUMN, 2 , (LPARAM)&xColumn );

	xColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM;
	xColumn.fmt = LVCFMT_RIGHT;
	xColumn.pszText = "Trigger?";
	xColumn.cx = 60;
	xColumn.iSubItem = 3;
	SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_INSERTCOLUMN, 3 , (LPARAM)&xColumn );

	xColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM;
	xColumn.fmt = LVCFMT_LEFT;
	xColumn.pszText = "Filename";
	xColumn.cx = 305;
	xColumn.iSubItem = 4;
	SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_INSERTCOLUMN, 4 , (LPARAM)&xColumn );

	SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_LIST1, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT );

	SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_ANIMATION_USE_LIST, CB_RESETCONTENT, 0, 0 );
	for ( nLoop = 0; nLoop < MAX_ANIMATION_USES; nLoop++ )
	{
		SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_ANIMATION_USE_LIST, CB_ADDSTRING, 0, (LPARAM)maszAnimationUses[nLoop] );
	}
	SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_ANIMATION_USE_LIST, CB_SETCURSEL, 0, 0 );
	
}


void  AnimationBuilderAddModelKeyframe( void )
{
int		nHandle;

	nHandle = AnimationBuilderAddModelKeyframeDlg();
	if ( nHandle != NOTFOUND )
	{
		AnimationBuilderAddNewKeyframe( nHandle );
	}
}



/***************************************************************************
 * Function    : AnimationBuilderDlgProc
 * Params      :
 * Returns     :
 * Description :
 ***************************************************************************/
LRESULT CALLBACK AnimationBuilderDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
short wNotifyCode;
int		nVal;
LPNMHDR		pNotifyHeader;
int		nSelected;

	switch (message)
	{
	case WM_INITDIALOG:
		mhwndAnimationBuilderDialog = hDlg;
		AnimationBuilderInitLists();
		mnAnimationHandle = ModelConvGetCurrentModel();
		if ( mnAnimationHandle != NOTFOUND )
		{
			AnimationBuilderExistingKeyframes( mnAnimationHandle );
		}
		SetWindowPos( mhwndAnimationBuilderDialog, NULL, 100, 100, 0,0, SWP_NOSIZE | SWP_NOZORDER );

		return TRUE;
	case WM_NOTIFY:
		pNotifyHeader = (LPNMHDR) lParam; 
		wNotifyCode = pNotifyHeader->code;
		switch ( wNotifyCode )
		{
		case NM_CLICK:
			nSelected = SendDlgItemMessage( hDlg, IDC_LIST1, LVM_GETSELECTIONMARK, 0, 0 );
			if ( nSelected != -1 )
			{
				mnSelectedKeyFrameNum = nSelected;
				AnimationBuilderEditKeyframe();
			}
			break;
		}
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); 
		switch ( wNotifyCode )
		{
		case CBN_SELCHANGE:
			if ( mnSelectedKeyFrameNum != NOTFOUND )
			{
				nVal = SendDlgItemMessage( hDlg, IDC_ANIMATION_USE_LIST, CB_GETCURSEL, 0, 0 );
				if ( maxBuilderKeyframeData[ mnSelectedKeyFrameNum ].bAnimationUse != nVal )
				{
					maxBuilderKeyframeData[ mnSelectedKeyFrameNum ].bAnimationUse = (byte)( nVal );
					AnimationBuilderUpdateList();
				}
			}
			break;

		case EN_CHANGE:
			{
			char	acString[256];
				GetDlgItemText( hDlg, IDC_KEYFRAME_TIME, acString, 256);
				nVal = strtol( acString, NULL, 10 );
				if ( nVal != maxBuilderKeyframeData[ mnSelectedKeyFrameNum ].uwKeyframeTime )
				{
					maxBuilderKeyframeData[ mnSelectedKeyFrameNum ].uwKeyframeTime = (ushort)( nVal );
					AnimationBuilderUpdateList();
				}
			}
			break;
		case BN_CLICKED:
			switch( LOWORD(wParam) )
			{
			case IDC_BUTTON2:
				AnimationBuilderAddModelSequence();
				break;
			case IDC_BUTTON1:
				AnimationBuilderAddModelKeyframe();
				break;
			case IDC_REMOVE_KEYFRAME:
				AnimationBuilderRemoveKeyframe();
				break;
			case IDC_CHECK1:
				if ( mnSelectedKeyFrameNum != NOTFOUND )
				{
					nVal = SendDlgItemMessage( mhwndAnimationBuilderDialog, IDC_CHECK1, BM_GETCHECK, 0, 0 );
					if ( nVal == BST_CHECKED )
					{
						if ( maxBuilderKeyframeData[ mnSelectedKeyFrameNum ].bAnimationTriggerCode == 0 )
						{
							maxBuilderKeyframeData[ mnSelectedKeyFrameNum ].bAnimationTriggerCode = 1;
							AnimationBuilderUpdateList();
						}
					}
					else
					{
						if ( maxBuilderKeyframeData[ mnSelectedKeyFrameNum ].bAnimationTriggerCode != 0 )
						{
							maxBuilderKeyframeData[ mnSelectedKeyFrameNum ].bAnimationTriggerCode = 0;
							AnimationBuilderUpdateList();
						}
					}
				}
				break;
			case IDCANCEL:
				if ( AnimationBuilderValidateKeyframes() == TRUE )
				{
				int		nHandleResult;

					nHandleResult = AnimationBuilderBuildKeyframes();
					if ( mnAnimationHandle != NOTFOUND )
					{
						ModelFree( mnAnimationHandle );
					}
					ModelConvSetCurrentModel( nHandleResult );
				}
				mnAnimationHandle = NOTFOUND;
				AnimationBuilderFreeAll();
				EndDialog(hDlg, 0);
				break;
			case IDOK:
				if ( AnimationBuilderValidateKeyframes() == TRUE )
				{
				int		nHandleResult;

					nHandleResult = AnimationBuilderBuildKeyframes();
					if ( mnAnimationHandle != NOTFOUND )
					{
						ModelFree( mnAnimationHandle );
					}
					ModelConvSetCurrentModel( nHandleResult );
					mnAnimationHandle = nHandleResult;
					AnimationBuilderExistingKeyframes( mnAnimationHandle );
					AnimationBuilderUpdateList();
				}
				else
				{

				}
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



