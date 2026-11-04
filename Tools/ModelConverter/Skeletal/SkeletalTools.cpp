
#include <stdio.h>
#include <windows.h>

#include <StandardDef.h>
#include <System.h>
#include <Engine.h>
#include <Rendering.h>

#include "../../../Pub/LibCode/Engine/FileFormats/fbxAnimation.h"

#include "SkeletalTools.h"

//-------------------------------------------------------------------------
// Pulls the next (optionally quoted) argument off the command line
//-------------------------------------------------------------------------
static const char*	SkeletalToolsNextArg( const char* pcRunner, char* szOut, int nMaxLen )
{
int		nLen = 0;
char	cEnd = ' ';

	while ( *pcRunner == ' ' ) pcRunner++;
	if ( *pcRunner == '\"' )
	{
		cEnd = '\"';
		pcRunner++;
	}
	while ( ( *pcRunner != 0 ) && ( *pcRunner != cEnd ) )
	{
		if ( nLen < nMaxLen - 1 ) szOut[nLen++] = *pcRunner;
		pcRunner++;
	}
	if ( *pcRunner == cEnd ) pcRunner++;
	szOut[nLen] = 0;
	return( pcRunner );
}

//-------------------------------------------------------------------------
// Function    : SkeletalToolsHandleCommandLine
// Description : Returns TRUE if the command line was a skeletal tool command (and has been run)
//-------------------------------------------------------------------------
BOOL		SkeletalToolsHandleCommandLine( const char* szCommandLine, int* pnExitCodeOut )
{
char		acCommand[64];
char		acIn[MAX_PATH];
char		acOut[MAX_PATH];
char		acFPS[32];
char		acError[512];
const char*	pcRunner;
BOOL		bResult;

	pcRunner = SkeletalToolsNextArg( szCommandLine, acCommand, 64 );
	if ( ( tinstricmp( acCommand, "-fbxanim2bvh" ) != 0 ) && ( tinstricmp( acCommand, "-bvh2fbx" ) != 0 ) )
	{
		return( FALSE );
	}
	pcRunner = SkeletalToolsNextArg( pcRunner, acIn, MAX_PATH );
	pcRunner = SkeletalToolsNextArg( pcRunner, acOut, MAX_PATH );
	pcRunner = SkeletalToolsNextArg( pcRunner, acFPS, 32 );
	acError[0] = 0;

	if ( ( acIn[0] == 0 ) || ( acOut[0] == 0 ) )
	{
		SysDebugPrint( "Usage: -fbxanim2bvh <in.fbx> <out.bvh> [fps]   or   -bvh2fbx <in.bvh> <out.fbx>\n" );
		*pnExitCodeOut = 2;
		return( TRUE );
	}

	if ( tinstricmp( acCommand, "-fbxanim2bvh" ) == 0 )
	{
		bResult = FbxAnimationConvertToBVH( acIn, acOut, ( acFPS[0] != 0 ) ? (float)atof( acFPS ) : 30.0f, acError, 512 );
	}
	else
	{
		bResult = FbxAnimationCreateFromBVH( acIn, acOut, acError, 512 );
	}
	if ( bResult == FALSE )
	{
		SysDebugPrint( "%s failed: %s\n", acCommand, acError );
	}
	*pnExitCodeOut = bResult ? 0 : 1;
	return( TRUE );
}

//-------------------------------------------------------------------------
// Function    : SkeletalToolsImportFBXAnimation
// Description : Menu - converts an FBX animation to .bvh (saved next to the source by default)
//-------------------------------------------------------------------------
void		SkeletalToolsImportFBXAnimation( void )
{
char	acCurrentDir[MAX_PATH];
char	acFBXFile[MAX_PATH] = "";
char	acBVHFile[MAX_PATH] = "";
char	acError[512];
char	acMessage[1024];

	SysGetCurrentDir( MAX_PATH, acCurrentDir );
	if ( SysGetOpenFilenameDialog( "Autodesk FBX (*.fbx)\0*.fbx\0All Files(*.*)\0*.*\0", "Import FBX animation", acCurrentDir, 0, acFBXFile ) == FALSE )
	{
		return;
	}
	strcpy( acBVHFile, acFBXFile );
	SysRemoveFileExtension( acBVHFile, "fbx" );
	SysAddFileExtensionIfNeeded( acBVHFile, "bvh" );
	if ( SysGetSaveFilenameDialog( "Biovision Hierarchy (*.bvh)\0*.bvh\0", "Save animation as .bvh (put it in the same folder as the .atm that uses it)", acCurrentDir, 0, acBVHFile ) == FALSE )
	{
		return;
	}
	SysAddFileExtensionIfNeeded( acBVHFile, "bvh" );

	if ( FbxAnimationConvertToBVH( acFBXFile, acBVHFile, 30.0f, acError, 512 ) )
	{
		sprintf( acMessage, "Saved %s\n\nUse Animations > Skeletal > Link Animation to add it to a rigged model.", acBVHFile );
		SysMessageBox( acMessage, "FBX animation import", SYSMESSBOX_OK );
	}
	else
	{
		SysMessageBox( acError, "FBX animation import failed", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
	}
}

void		SkeletalToolsExportBVHAsFBX( void )
{
char	acCurrentDir[MAX_PATH];
char	acBVHFile[MAX_PATH] = "";
char	acFBXFile[MAX_PATH] = "";
char	acError[512];

	SysGetCurrentDir( MAX_PATH, acCurrentDir );
	if ( SysGetOpenFilenameDialog( "Biovision Hierarchy (*.bvh)\0*.bvh\0All Files(*.*)\0*.*\0", "Select .bvh animation", acCurrentDir, 0, acBVHFile ) == FALSE )
	{
		return;
	}
	strcpy( acFBXFile, acBVHFile );
	SysRemoveFileExtension( acFBXFile, "bvh" );
	SysAddFileExtensionIfNeeded( acFBXFile, "fbx" );
	if ( SysGetSaveFilenameDialog( "Autodesk FBX (*.fbx)\0*.fbx\0", "Save as FBX", acCurrentDir, 0, acFBXFile ) == FALSE )
	{
		return;
	}
	SysAddFileExtensionIfNeeded( acFBXFile, "fbx" );
	if ( FbxAnimationCreateFromBVH( acBVHFile, acFBXFile, acError, 512 ) == FALSE )
	{
		SysMessageBox( acError, "BVH to FBX failed", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
	}
}

//-------------------------------------------------------------------------
// Function    : SkeletalToolsAutoRigHumanoid
// Description : Menu - one-click rig. Assumes a humanoid standing Z-up, facing +X, arms down
//-------------------------------------------------------------------------
void		SkeletalToolsAutoRigHumanoid( int hModel )
{
VECT	axJoints[HUMANOID_NUM_BONES];
char	acMessage[512];

	if ( hModel == NOTFOUND ) return;

	ModelRemoveVertexAnimation( hModel );
	ModelSkeletonEstimateHumanoidJoints( hModel, axJoints );
	if ( ModelSkeletonCreateHumanoid( hModel, axJoints ) == FALSE )
	{
		SysMessageBox( "Couldn't create the skeleton", "Auto-rig", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
		return;
	}
	ModelSkeletonAutoGenerateWeights( hModel, 2 );

	sprintf( acMessage, "Rigged with the standard humanoid skeleton (%d bones, estimated joint positions, automatic weights).\n\n"
						"Any vertex-keyframe animation has been removed.\n"
						"Next: Animations > Skeletal > Link Animation, then Save As .atm", ModelSkeletonGetNumBones( hModel ) );
	SysMessageBox( acMessage, "Auto-rig", SYSMESSBOX_OK );
}

//-------------------------------------------------------------------------
// Function    : SkeletalToolsLinkAnimation
// Description : Menu - links a .bvh into the model. The animation use is guessed from the filename
//-------------------------------------------------------------------------
void		SkeletalToolsLinkAnimation( int hModel )
{
char	acCurrentDir[MAX_PATH];
char	acBVHFile[MAX_PATH] = "";
char	acLower[MAX_PATH];
char	acMessage[512];
int		nAnimUse = ANIM_RUN;

	if ( hModel == NOTFOUND ) return;
	if ( ModelHasSkeleton( hModel ) == FALSE )
	{
		SysMessageBox( "The model has no skeleton - use Auto-rig first", "Link animation", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
		return;
	}

	SysGetCurrentDir( MAX_PATH, acCurrentDir );
	if ( SysGetOpenFilenameDialog( "Biovision Hierarchy (*.bvh)\0*.bvh\0", "Link animation (.bvh, from the model's folder)", acCurrentDir, 0, acBVHFile ) == FALSE )
	{
		return;
	}

	StringToLower( acBVHFile, acLower );
	if ( strstr( acLower, "walk" ) ) nAnimUse = ANIM_WALK;
	else if ( strstr( acLower, "idle" ) || strstr( acLower, "stand" ) ) nAnimUse = ANIM_STANDING;
	else if ( strstr( acLower, "jump" ) ) nAnimUse = ANIM_JUMP;
	else if ( strstr( acLower, "die" ) || strstr( acLower, "death" ) ) nAnimUse = ANIM_DIE;
	else if ( strstr( acLower, "wave" ) ) nAnimUse = ANIM_WAVE;

	if ( ModelSkeletonAddAnimation( hModel, acBVHFile, nAnimUse, TRUE ) == NOTFOUND )
	{
		SysMessageBox( "Couldn't load that animation", "Link animation", SYSMESSBOX_OK | SYSMESSBOX_EXCLAMATION );
		return;
	}
	ModelSetAnimationImmediate( hModel, nAnimUse, 1, NULL, 0 );

	sprintf( acMessage, "Linked as '%s'.\n\nOnly the filename is stored in the .atm - keep the .bvh in the same folder as the saved model.", ModelAnimationGetName( nAnimUse ) );
	SysMessageBox( acMessage, "Link animation", SYSMESSBOX_OK );
}
