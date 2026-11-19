
#include <stdio.h>
#include <windows.h>

#include <StandardDef.h>
#include <System.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#include "../../../Pub/LibCode/Engine/ModelRendering.h"

#include "../resource.h"
#include "../ModelConverter.h"
#include "SkeletalView.h"

extern HWND		mhwndMainDialog;		// ModelConverter.cpp

static BOOL		msbSkeletalViewShowBones = FALSE;
static BOOL		msbSkeletalViewShowBindPose = FALSE;
static BOOL		msbSkeletalViewInVertexEdit = FALSE;
static int		msnSkeletalViewComboModel = NOTFOUND;		// The model whose links are listed in the animation combo
static int		msnSkeletalViewComboNumAnims = 0;
static int		msnSkeletalViewComboSelection = NOTFOUND;
static int		msnSkeletalViewCommandModel = NOTFOUND;
static int		msnSkeletalViewWarnedModel = NOTFOUND;

//-------------------------------------------------------------------------

static BOOL		SkeletalViewModelIsSkinned( int hModel )
{
	return( ( hModel != NOTFOUND ) && ( ModelHasSkeleton( hModel ) ) );
}

static void		SkeletalViewApplyBindPoseSetting( int hModel )
{
	if ( SkeletalViewModelIsSkinned( hModel ) )
	{
		ModelSkeletonSetShowBindPose( hModel, ( msbSkeletalViewShowBindPose || msbSkeletalViewInVertexEdit ) ? TRUE : FALSE );
	}
}

void	SkeletalViewSetShowBones( BOOL bFlag )
{
	msbSkeletalViewShowBones = bFlag;
}

BOOL	SkeletalViewGetShowBones( void )
{
	return( msbSkeletalViewShowBones );
}

void	SkeletalViewSetShowBindPose( BOOL bFlag )
{
	msbSkeletalViewShowBindPose = bFlag;
}

BOOL	SkeletalViewGetShowBindPose( void )
{
	return( msbSkeletalViewShowBindPose );
}

//-------------------------------------------------------------------------
// Function    : SkeletalViewFillAnimCombo
// Description : Lists the model's linked animations in the toolbar's animation combo
//-------------------------------------------------------------------------
static void		SkeletalViewFillAnimCombo( int hModel )
{
MODEL_SKELETON_ANIM_INFO	xInfo;
char	acString[256];
int		nNumAnims = ModelSkeletonGetNumAnimations( hModel );
int		nLoop;

	SendDlgItemMessage( mhwndMainDialog, IDC_COMBO1, CB_RESETCONTENT, 0, 0 );
	for ( nLoop = 0; nLoop < nNumAnims; nLoop++ )
	{
		ModelSkeletonGetAnimationInfo( hModel, nLoop, &xInfo );
		_snprintf( acString, sizeof( acString ) - 1, "%s - %s%s", ModelAnimationGetName( xInfo.nAnimUse ), xInfo.acFilename, xInfo.bLoaded ? "" : " (missing)" );
		acString[sizeof( acString ) - 1] = 0;
		SendDlgItemMessage( mhwndMainDialog, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)acString );
	}
	msnSkeletalViewComboModel = hModel;
	msnSkeletalViewComboNumAnims = nNumAnims;
	msnSkeletalViewComboSelection = NOTFOUND;
}

int		SkeletalViewUpdateAnimControls( int hModel, BOOL bVertexEditMode )
{
int		nCurrent;

	// Vertex editing (picker modes) works on the bind pose. When it ends, the edited mesh becomes the bind pose
	if ( bVertexEditMode != msbSkeletalViewInVertexEdit )
	{
		msbSkeletalViewInVertexEdit = bVertexEditMode;
		if ( ( bVertexEditMode == FALSE ) && ( SkeletalViewModelIsSkinned( hModel ) ) )
		{
			ModelSkeletonRecaptureBindPose( hModel );
		}
	}
	SkeletalViewApplyBindPoseSetting( hModel );

	if ( ( SkeletalViewModelIsSkinned( hModel ) == FALSE ) ||
		 ( ModelSkeletonGetNumAnimations( hModel ) == 0 ) )
	{
		if ( msnSkeletalViewComboModel != NOTFOUND )
		{
			msnSkeletalViewComboModel = NOTFOUND;
			return( -1 );
		}
		return( 0 );
	}

	if ( ( msnSkeletalViewComboModel != hModel ) ||
		 ( msnSkeletalViewComboNumAnims != ModelSkeletonGetNumAnimations( hModel ) ) )
	{
		SkeletalViewFillAnimCombo( hModel );
	}
	nCurrent = ModelSkeletonGetCurrentAnimation( hModel );
	if ( nCurrent != msnSkeletalViewComboSelection )
	{
		SendDlgItemMessage( mhwndMainDialog, IDC_COMBO1, CB_SETCURSEL, ( nCurrent == NOTFOUND ) ? 0 : nCurrent, 0 );
		msnSkeletalViewComboSelection = nCurrent;
	}

	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIMTEXT ), TRUE );
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_COMBO1 ), TRUE );
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIM_STOP ), TRUE );
	EnableWindow( GetDlgItem( mhwndMainDialog, IDC_ANIM_PLAY ), TRUE );
	return( 1 );
}

BOOL	SkeletalViewOnAnimComboSelect( int hModel, int nSelection )
{
	if ( ( msnSkeletalViewComboModel == NOTFOUND ) ||
		 ( msnSkeletalViewComboModel != hModel ) ||
		 ( SkeletalViewModelIsSkinned( hModel ) == FALSE ) )
	{
		return( FALSE );
	}
	if ( ( nSelection >= 0 ) && ( nSelection < ModelSkeletonGetNumAnimations( hModel ) ) )
	{
		ModelSkeletonPlayAnimationSlot( hModel, nSelection );
		ModelAnimationPause( hModel, FALSE );		// as choosing a vertex-keyframe animation does
	}
	return( TRUE );
}

//-------------------------------------------------------------------------
// Function    : SkeletalViewDrawOverlay
// Description : Bones and joints (drawn over the mesh, so they show through it) and the animation's name and time
//-------------------------------------------------------------------------
void	SkeletalViewDrawOverlay( int hModel, fnSkeletalViewProject fnProject )
{
VECT	axJoints[MODEL_SKELETON_MAX_BONES];
int		anScreenX[MODEL_SKELETON_MAX_BONES];
int		anScreenY[MODEL_SKELETON_MAX_BONES];
const MODEL_SKELETON_BONE*	pxBone;
MODEL_SKELETON_ANIM_INFO	xInfo;
char	acString[256];
int		nNumBones;
int		nLoop;
int		nCurrent;

	if ( SkeletalViewModelIsSkinned( hModel ) == FALSE ) return;

	if ( msbSkeletalViewShowBones )
	{
		nNumBones = ModelSkeletonGetCurrentJointPositions( hModel, axJoints, MODEL_SKELETON_MAX_BONES );
		for ( nLoop = 0; nLoop < nNumBones; nLoop++ )
		{
			fnProject( &axJoints[nLoop], &anScreenX[nLoop], &anScreenY[nLoop] );
		}
		for ( nLoop = 0; nLoop < nNumBones; nLoop++ )
		{
			pxBone = ModelSkeletonGetBone( hModel, nLoop );
			if ( ( pxBone ) && ( pxBone->nParentIndex != NOTFOUND ) )
			{
				InterfaceLine( 1, anScreenX[pxBone->nParentIndex], anScreenY[pxBone->nParentIndex], anScreenX[nLoop], anScreenY[nLoop], 0xFF40FF60, 0xFF40FF60 );
			}
		}
		for ( nLoop = 0; nLoop < nNumBones; nLoop++ )
		{
			InterfaceRect( 1, anScreenX[nLoop] - 2, anScreenY[nLoop] - 2, 5, 5, ( nLoop == 0 ) ? 0xFFFFE040 : 0xFFF0F0F0 );
		}
	}

	// Which animation, and where in it
	if ( ModelSkeletonGetNumAnimations( hModel ) > 0 )
	{
		nCurrent = ModelSkeletonGetCurrentAnimation( hModel );
		if ( ( msbSkeletalViewShowBindPose ) || ( msbSkeletalViewInVertexEdit ) )
		{
			_snprintf( acString, sizeof( acString ) - 1, "Bind pose" );
		}
		else if ( ( nCurrent != NOTFOUND ) && ( ModelSkeletonGetAnimationInfo( hModel, nCurrent, &xInfo ) ) )
		{
			if ( xInfo.bLoaded )
			{
				_snprintf( acString, sizeof( acString ) - 1, "%s - %s   %.2f / %.2fs%s", ModelAnimationGetName( xInfo.nAnimUse ), xInfo.acFilename,
						   ModelSkeletonGetAnimationTime( hModel ), ( xInfo.nLoopEnd - xInfo.nLoopStart ) * xInfo.fFrameTime,
						   maxModelRenderData[hModel].xAnimationState.boIsPaused ? "   (paused)" : "" );
			}
			else
			{
				_snprintf( acString, sizeof( acString ) - 1, "%s - %s   (file missing: bind pose)", ModelAnimationGetName( xInfo.nAnimUse ), xInfo.acFilename );
			}
		}
		else
		{
			acString[0] = 0;
		}
		acString[sizeof( acString ) - 1] = 0;
		InterfaceSetFontFlags( FONT_FLAG_SMALL );
		InterfaceText( 1, 10, 24, acString, 0xFFE0E0E0, 0 );
		InterfaceSetFontFlags( 0 );
	}
}

//-------------------------------------------------------------------------
// Function    : SkeletalViewReportModel
// Description : Lists the skeleton and its animation links in the output window, and warns about missing files
//-------------------------------------------------------------------------
void	SkeletalViewReportModel( int hModel )
{
MODEL_SKELETON_ANIM_INFO	xInfo;
const MODEL_SKIN_WEIGHT*	pxWeights;
char	acMissing[1024] = "";
char	acMessage[1536];
int		nNumWeights = 0;
int		nNumAnims;
int		nNumMissing = 0;
int		nLoop;

	if ( SkeletalViewModelIsSkinned( hModel ) == FALSE ) return;

	pxWeights = ModelSkeletonGetVertexWeights( hModel, &nNumWeights );
	nNumAnims = ModelSkeletonGetNumAnimations( hModel );
	ModelConvTextAdd( "   Skeleton: %d bones, %d vertex weights%s, %d linked animations", ModelSkeletonGetNumBones( hModel ), nNumWeights,
					  ( ( pxWeights != NULL ) && ( nNumWeights == ModelGetStats( hModel )->nNumVertices ) ) ? "" : " (don't match the vertices)", nNumAnims );
	for ( nLoop = 0; nLoop < nNumAnims; nLoop++ )
	{
		ModelSkeletonGetAnimationInfo( hModel, nLoop, &xInfo );
		if ( xInfo.bLoaded )
		{
			ModelConvTextAdd( "      %s: %s (%d frames at %.0f fps, loop %d-%d)", ModelAnimationGetName( xInfo.nAnimUse ), xInfo.acFilename, xInfo.nNumFrames,
							  ( xInfo.fFrameTime > 0.0f ) ? ( 1.0f / xInfo.fFrameTime ) : 0.0f, xInfo.nLoopStart, xInfo.nLoopEnd );
		}
		else
		{
			ModelConvTextAdd( "      %s: %s - NOT FOUND (the link is kept and saved)", ModelAnimationGetName( xInfo.nAnimUse ), xInfo.acFilename );
			if ( strlen( acMissing ) + strlen( xInfo.acFilename ) + 8 < sizeof( acMissing ) )
			{
				strcat( acMissing, "   " );
				strcat( acMissing, xInfo.acFilename );
				strcat( acMissing, "\n" );
			}
			nNumMissing++;
		}
	}
	if ( nNumMissing > 0 )
	{
		_snprintf( acMessage, sizeof( acMessage ) - 1, "%d linked animation file(s) weren't found next to the model:\n\n%s\n"
					"The links are kept, and saved with the model. Put the .bvh files in the model's folder to see them play.", nNumMissing, acMissing );
		acMessage[sizeof( acMessage ) - 1] = 0;
		SysMessageBox( acMessage, "Skeletal animations", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
	}
}

//-------------------------------------------------------------------------
// Function    : SkeletalViewBeginCommand / SkeletalViewEndCommand
// Description : Every command runs with the skinned model in its bind pose (held there, in case the command opens a dialog
//				 while frames keep drawing). Afterwards the bind pose is taken from the mesh again, so edits stick.
//-------------------------------------------------------------------------
void	SkeletalViewBeginCommand( int hModel )
{
	msnSkeletalViewCommandModel = NOTFOUND;
	if ( SkeletalViewModelIsSkinned( hModel ) == FALSE ) return;

	msnSkeletalViewCommandModel = hModel;
	ModelSkeletonSetShowBindPose( hModel, TRUE );
	ModelSkeletonRestoreBindPose( hModel );
}

void	SkeletalViewEndCommand( int hModel )
{
int		nNumWeights = 0;

	if ( ( msnSkeletalViewCommandModel != NOTFOUND ) &&
		 ( msnSkeletalViewCommandModel == hModel ) &&
		 ( SkeletalViewModelIsSkinned( hModel ) ) )
	{
		ModelSkeletonGetVertexWeights( hModel, &nNumWeights );
		if ( nNumWeights != ModelGetStats( hModel )->nNumVertices )
		{
			if ( msnSkeletalViewWarnedModel != hModel )
			{
				msnSkeletalViewWarnedModel = hModel;
				SysMessageBox( "That changed the number of vertices, so the skin weights no longer fit and the model won't animate.\n\n"
							   "Use Skeleton > Auto-rig as Humanoid to rig it again, or reload the model.", "Skinned model", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
			}
		}
		else
		{
			ModelSkeletonRecaptureBindPose( hModel );
		}
	}
	msnSkeletalViewCommandModel = NOTFOUND;
	SkeletalViewApplyBindPoseSetting( hModel );
}

//-------------------------------------------------------------------------

static BOOL		SkeletalViewGetFileSize( const char* szFilename, DWORD* pulSizeOut )
{
WIN32_FILE_ATTRIBUTE_DATA	xData;

	if ( GetFileAttributesEx( szFilename, GetFileExInfoStandard, &xData ) == FALSE ) return( FALSE );
	*pulSizeOut = xData.nFileSizeLow;
	return( TRUE );
}

BOOL	SkeletalViewCopyAnimationToModelFolder( const char* szAnimFile, const char* szModelFilename, char* szPathOut, int nPathOutLen )
{
char	acModelFolder[MAX_PATH];
char	acAnimFolder[MAX_PATH];
char	acName[MAX_PATH];
char	acDest[MAX_PATH];
char	acMessage[MAX_PATH * 2 + 256];
DWORD	ulSrcSize = 0;
DWORD	ulDestSize = 0;

	strncpy( szPathOut, szAnimFile, nPathOutLen - 1 );
	szPathOut[nPathOutLen - 1] = 0;

	SysExtractPathFromFilename( szModelFilename, acModelFolder );
	SysExtractPathFromFilename( szAnimFile, acAnimFolder );
	if ( ( acModelFolder[0] == 0 ) || ( tinstricmp( acModelFolder, acAnimFolder ) == 0 ) )
	{
		return( TRUE );
	}
	SysExtractFilenameFromPath( szAnimFile, acName );
	_snprintf( acDest, sizeof( acDest ) - 1, "%s\\%s", acModelFolder, acName );
	acDest[sizeof( acDest ) - 1] = 0;

	if ( SkeletalViewGetFileSize( acDest, &ulDestSize ) )
	{
		SkeletalViewGetFileSize( szAnimFile, &ulSrcSize );
		if ( ulSrcSize != ulDestSize )
		{
			_snprintf( acMessage, sizeof( acMessage ) - 1, "%s already exists and is different from\n%s\n\nReplace it?", acDest, szAnimFile );
			acMessage[sizeof( acMessage ) - 1] = 0;
			if ( SysMessageBox( acMessage, "Copy animation", SYSMESSBOX_YESNO ) == FALSE )
			{
				strncpy( szPathOut, acDest, nPathOutLen - 1 );		// use the one that's there
				szPathOut[nPathOutLen - 1] = 0;
				return( TRUE );
			}
		}
		else
		{
			strncpy( szPathOut, acDest, nPathOutLen - 1 );
			szPathOut[nPathOutLen - 1] = 0;
			return( TRUE );
		}
	}
	if ( CopyFile( szAnimFile, acDest, FALSE ) == FALSE )
	{
		ModelConvTextAdd( "Couldn't copy %s to %s", szAnimFile, acDest );
		return( FALSE );
	}
	ModelConvTextAdd( "Copied %s to %s", acName, acModelFolder );
	strncpy( szPathOut, acDest, nPathOutLen - 1 );
	szPathOut[nPathOutLen - 1] = 0;
	return( TRUE );
}

//-------------------------------------------------------------------------
// Function    : SkeletalViewCopyLinkedAnimations
// Description : Links are only filenames, relative to the model, so the files go next to the saved .atm
//-------------------------------------------------------------------------
void	SkeletalViewCopyLinkedAnimations( int hModel, const char* szNewModelFilename )
{
MODEL_SKELETON_ANIM_INFO	xInfo;
char	acNewFolder[MAX_PATH];
char	acDest[MAX_PATH];
char	acUsed[MAX_PATH];
char	acMissing[1024] = "";
char	acMessage[1536];
int		nNumAnims;
int		nNumMissing = 0;
int		nLoop;

	if ( SkeletalViewModelIsSkinned( hModel ) == FALSE ) return;
	nNumAnims = ModelSkeletonGetNumAnimations( hModel );
	SysExtractPathFromFilename( szNewModelFilename, acNewFolder );

	for ( nLoop = 0; nLoop < nNumAnims; nLoop++ )
	{
		ModelSkeletonGetAnimationInfo( hModel, nLoop, &xInfo );
		if ( xInfo.bLinkInModel == FALSE ) continue;

		if ( xInfo.acLoadedFrom[0] != 0 )
		{
			SkeletalViewCopyAnimationToModelFolder( xInfo.acLoadedFrom, szNewModelFilename, acUsed, MAX_PATH );
		}
		else
		{
			// Not loaded: fine if it's already where the new model will look for it
			_snprintf( acDest, sizeof( acDest ) - 1, "%s\\%s", acNewFolder, xInfo.acFilename );
			acDest[sizeof( acDest ) - 1] = 0;
			if ( SysFileDoesExist( acDest ) == FALSE )
			{
				if ( strlen( acMissing ) + strlen( xInfo.acFilename ) + 8 < sizeof( acMissing ) )
				{
					strcat( acMissing, "   " );
					strcat( acMissing, xInfo.acFilename );
					strcat( acMissing, "\n" );
				}
				nNumMissing++;
			}
		}
	}
	if ( nNumMissing > 0 )
	{
		_snprintf( acMessage, sizeof( acMessage ) - 1, "Saved, but %d linked animation file(s) aren't next to the new model:\n\n%s\n"
					"The links are saved. Copy the files to %s to see them play.", nNumMissing, acMissing, acNewFolder );
		acMessage[sizeof( acMessage ) - 1] = 0;
		SysMessageBox( acMessage, "Skeletal animations", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
	}
}
