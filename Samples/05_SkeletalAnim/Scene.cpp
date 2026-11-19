
#include <StdWinInclude.h>
#include <d3dx9.h>
#include <StandardDef.h>
#include <Interface.h>
#include <Engine.h>
#include <Rendering.h>

#include "../../Pub/LibCode/Engine/ModelFiles.h"		// for ModelExportATM

#include "FrameCapture.h"
#include "Scene.h"

//-----------------------------------------------------------------------------------------------------------------------
// Scene modes (from the command line)
//
//  -dump		Writes info + .obj of caveman1.atm and some reference screenshots to Data\Samples\SkelOut
//  -rig		Converts caveman1.atm into a skinned model : removes the vertex animation, fits the standard humanoid
//				skeleton, auto-generates weights, links cmu_09_01_run.bvh as ANIM_RUN, and saves caveman1_skinned.atm
//  -video		Renders the original and skinned caveman side by side to Data\Samples\SkelOut\caveman_skeletal_run.mp4
//  -karate		Renders the skinned caveman performing CMU 135_02 ('Empi' karate kata, with root motion) to
//				Data\Samples\SkelOut\caveman_karate_empi.mp4 (can be combined with -rig)
//  -shots		Saves a few stills of the skinned run to Data\Samples\SkelOut (for checking without watching a video)
//  (none)		Interactive viewer. B = toggle bone overlay, O = toggle original model
//-----------------------------------------------------------------------------------------------------------------------

#define		SKELOUT_FOLDER			"Data\\Samples\\SkelOut"
#define		ORIGINAL_MODEL			"Data\\Samples\\caveman1.atm"
#define		SKINNED_MODEL			"Data\\Samples\\caveman1_skinned.atm"
#define		RUN_ANIMATION			"cmu_09_01_run.bvh"		// (relative to the model's folder)
#define		KARATE_ANIMATION		"cmu_135_02_karate.bvh"

#define		VIDEO_FPS				30
#define		VIDEO_NUM_FRAMES		(VIDEO_FPS * 12)
#define		KARATE_VIDEO_NUM_FRAMES	(VIDEO_FPS * 43)		// the whole kata (5203 frames @ 120fps)

enum
{
	SCENE_MODE_INTERACTIVE,
	SCENE_MODE_DUMP_SHOTS,
	SCENE_MODE_VIDEO,
	SCENE_MODE_STILLS,
};

static int		msnSceneMode = SCENE_MODE_INTERACTIVE;
static int		mshOriginalModel = NOTFOUND;
static int		mshSkinnedModel = NOTFOUND;
static int		msnFrameNum = 0;
static BOOL		msbShowBones = FALSE;
static BOOL		msbShowOriginal = TRUE;
static float	msfCameraAngle = -0.6f;
static VECT		msxCamPos;
static VECT		msxCamTarget;
static BOOL		msbKarateMode = FALSE;
static VECT		msxFollowTarget = { 0.0f, 0.0f, 0.85f };

static const VECT	mxOriginalModelPos = { 0.0f, 0.55f, 0.0f };
static VECT			mxSkinnedModelPos = { 0.0f, -0.55f, 0.0f };

//-----------------------------------------------------------------------------------------------------------------------
// Joint positions for caveman1.atm (model space : Z up, facing +X, character's left = +Y).
// Measured from vertex clusters around each joint in the base (standing) pose - see Pub\skeletalanimation.md
//-----------------------------------------------------------------------------------------------------------------------
static const VECT	maxCavemanJoints[HUMANOID_NUM_BONES] =
{
	{ 0.10f,  0.000f, 0.86f },		// Hips
	{ 0.09f,  0.000f, 1.00f },		// Spine
	{ 0.12f,  0.000f, 1.20f },		// Spine1
	{ 0.14f,  0.000f, 1.42f },		// Neck
	{ 0.16f,  0.000f, 1.50f },		// Head
	{ 0.11f,  0.070f, 1.36f },		// LeftShoulder
	{ 0.10f,  0.200f, 1.32f },		// LeftArm
	{ 0.10f,  0.250f, 0.99f },		// LeftForeArm
	{ 0.20f,  0.270f, 0.73f },		// LeftHand
	{ 0.11f, -0.070f, 1.36f },		// RightShoulder
	{ 0.12f, -0.200f, 1.32f },		// RightArm
	{ 0.03f, -0.235f, 1.01f },		// RightForeArm
	{ 0.075f,-0.250f, 0.745f },		// RightHand
	{ 0.11f,  0.095f, 0.82f },		// LeftUpLeg
	{ 0.17f,  0.135f, 0.505f },		// LeftLeg
	{ 0.05f,  0.165f, 0.075f },		// LeftFoot
	{ 0.17f,  0.170f, 0.02f },		// LeftToeBase
	{ 0.09f, -0.095f, 0.82f },		// RightUpLeg
	{ 0.165f,-0.127f, 0.50f },		// RightLeg
	{ 0.055f,-0.163f, 0.07f },		// RightFoot
	{ 0.17f, -0.170f, 0.02f },		// RightToeBase
};

//-----------------------------------------------------------------------------------------------------------------------

static void		SceneSetCamera( const VECT* pxPos, const VECT* pxTarget )
{
	msxCamPos = *pxPos;
	msxCamTarget = *pxTarget;
}

static void		SceneSetOrbitCamera( float fAngle, float fDist, float fHeight, float fTargetHeight )
{
VECT	xPos;
VECT	xTarget = { 0.0f, 0.0f, fTargetHeight };

	xPos.x = cosf( fAngle ) * fDist;
	xPos.y = sinf( fAngle ) * fDist;
	xPos.z = fHeight;
	SceneSetCamera( &xPos, &xTarget );
}

static void		SceneApplyCamera( void )
{
VECT	xDir;
VECT	xLightDir = { -0.6f, 0.8f, -0.7f };

	VectSub( &xDir, &msxCamTarget, &msxCamPos );
	EngineCameraSetPos( msxCamPos.x, msxCamPos.y, msxCamPos.z );
	EngineCameraSetDirection( xDir.x, xDir.y, xDir.z );
	EngineCameraSetUpVect( 0.0f, 0.0f, 1.0f );
	EngineCameraSetProjection( PI/4, 0.05f, 100.0f );
	EngineCameraUpdate();
	EngineDefaultState();

	// The models have no texture - light them, using the vertex colours as the material
	EngineSetTexture( 0, NOTFOUND );
	EngineSetColourMode( 0, COLOUR_MODE_DIFFUSE_ONLY );
	EngineActivateSimpleLight( 0, 0, 0xFFFFFFFF, &xLightDir );
	EngineEnableLighting( TRUE );
}

//-------------------------------------------------------------------------
// Function    : SceneDumpModel
// Description : Writes stats, keyframe info and the base mesh (as .obj) for analysis
//-------------------------------------------------------------------------
static void		SceneDumpModel( int hModel, const char* szName )
{
char			acFilename[256];
FILE*			pFile;
MODEL_STATS*	pxStats = ModelGetStats( hModel );
CUSTOMVERTEX*	pxVerts;
ushort*			puwIndices;
int				nLoop;

	sprintf( acFilename, SKELOUT_FOLDER "\\%s_info.txt", szName );
	pFile = fopen( acFilename, "wb" );
	if ( pFile == NULL ) return;

	fprintf( pFile, "Verts %d  Normals %d  Indices %d  Materials %d  VertKeyframes %d  Submodels %d\r\n",
				pxStats->nNumVertices, pxStats->nNumNormals, pxStats->nNumIndices, pxStats->nNumMaterials, pxStats->nNumVertKeyframes, pxStats->nNumSubmodels );
	fprintf( pFile, "BoundMin %f %f %f  BoundMax %f %f %f\r\n", pxStats->xBoundMin.x, pxStats->xBoundMin.y, pxStats->xBoundMin.z,
				pxStats->xBoundMax.x, pxStats->xBoundMax.y, pxStats->xBoundMax.z );
	for ( nLoop = 0; nLoop < MAX_ANIMATION_USES; nLoop++ )
	{
		if ( ModelAnimationIsPresent( hModel, nLoop ) )
		{
			fprintf( pFile, "Anim present: %d %s\r\n", nLoop, ModelAnimationGetName( nLoop ) );
		}
	}
	fprintf( pFile, "Skeleton bones %d, animations %d\r\n", ModelSkeletonGetNumBones( hModel ), ModelSkeletonGetNumAnimations( hModel ) );
	fclose( pFile );

	sprintf( acFilename, SKELOUT_FOLDER "\\%s_mesh.obj", szName );
	pFile = fopen( acFilename, "wb" );
	if ( pFile == NULL ) return;

	pxVerts = ModelLockVertexBuffer( hModel );
	for ( nLoop = 0; nLoop < pxStats->nNumVertices; nLoop++ )
	{
		fprintf( pFile, "v %f %f %f\r\n", pxVerts[nLoop].position.x, pxVerts[nLoop].position.y, pxVerts[nLoop].position.z );
	}
	ModelUnlockVertexBuffer( hModel );

	puwIndices = ModelLockIndexBuffer( hModel );
	for ( nLoop = 0; nLoop < pxStats->nNumIndices; nLoop += 3 )
	{
		fprintf( pFile, "f %d %d %d\r\n", puwIndices[nLoop] + 1, puwIndices[nLoop+1] + 1, puwIndices[nLoop+2] + 1 );
	}
	ModelUnlockIndexBuffer( hModel );
	fclose( pFile );

	// Skin weights, one line per vertex : bone:weight for each influence
	{
	int							nNumWeights = 0;
	const MODEL_SKIN_WEIGHT*	pxWeights = ModelSkeletonGetVertexWeights( hModel, &nNumWeights );
	int							nSlot;

		if ( pxWeights == NULL ) return;
		sprintf( acFilename, SKELOUT_FOLDER "\\%s_weights.txt", szName );
		pFile = fopen( acFilename, "wb" );
		if ( pFile == NULL ) return;
		for ( nLoop = 0; nLoop < nNumWeights; nLoop++ )
		{
			fprintf( pFile, "%d", nLoop );
			for ( nSlot = 0; nSlot < 4; nSlot++ )
			{
				if ( pxWeights[nLoop].afWeight[nSlot] > 0.0f )
				{
					fprintf( pFile, " %s:%.2f", ModelSkeletonGetBone( hModel, pxWeights[nLoop].abBoneIndex[nSlot] )->acName, pxWeights[nLoop].afWeight[nSlot] );
				}
			}
			fprintf( pFile, "\r\n" );
		}
		fclose( pFile );
	}
}

//-------------------------------------------------------------------------
// Function    : SceneRigCaveman
// Description : The 'pipeline' for making the skinned caveman, in code
//-------------------------------------------------------------------------
static BOOL		SceneRigCaveman( void )
{
int		hModel = ModelLoad( ORIGINAL_MODEL, 0, 1.0f );

	if ( hModel == NOTFOUND ) return( FALSE );

	// The base mesh is in the standing pose straight after loading - keep that as the bind pose
	ModelRemoveVertexAnimation( hModel );

	ModelSkeletonCreateHumanoid( hModel, maxCavemanJoints );
	ModelSkeletonAutoGenerateWeights( hModel, 2 );

	if ( ModelSkeletonAddAnimation( hModel, RUN_ANIMATION, ANIM_RUN, TRUE ) == NOTFOUND )
	{
		SysMessageBox( "Couldn't load the run animation " RUN_ANIMATION, "SkeletalAnim", SYSMESSBOX_OK );
	}

	ModelExportATM( hModel, SKINNED_MODEL, ATM_EXPORT_FLAGS_MATERIALS | ATM_EXPORT_FLAGS_COMPRESSION, 0 );
	ModelFree( hModel );

	// Reload it (exercises the new .atm chunks) and dump it to check
	hModel = ModelLoad( SKINNED_MODEL, 0, 1.0f );
	if ( hModel == NOTFOUND ) return( FALSE );
	SceneDumpModel( hModel, "caveman1_skinned" );
	ModelFree( hModel );
	return( TRUE );
}

//-------------------------------------------------------------------------
void		SceneInitialise( const char* szCommandLine )
{
	SysMkDir( SKELOUT_FOLDER );

	if ( strstr( szCommandLine, "-rig" ) )
	{
		SceneRigCaveman();
	}

	mshOriginalModel = ModelLoad( ORIGINAL_MODEL, 0, 1.0f );
	ModelSetVertexColours( mshOriginalModel, 0xFFB08868 );

	if ( strstr( szCommandLine, "-dump" ) )
	{
		SceneDumpModel( mshOriginalModel, "caveman1" );
		ModelAnimationPause( mshOriginalModel, TRUE );
		msnSceneMode = SCENE_MODE_DUMP_SHOTS;
		return;
	}

	mshSkinnedModel = ModelLoad( SKINNED_MODEL, 0, 1.0f );
	if ( mshSkinnedModel == NOTFOUND )
	{
		// Not built yet - make it now
		SceneRigCaveman();
		mshSkinnedModel = ModelLoad( SKINNED_MODEL, 0, 1.0f );
	}
	ModelSetVertexColours( mshSkinnedModel, 0xFFC89878 );

	// (Advance the render clock first - the keyframe animation code treats a start tick of 0 as 'not started')
	ModelRenderingUpdate( 1000 );
	ModelSetAnimationImmediate( mshOriginalModel, ANIM_RUN, 1, NULL, 0 );
	ModelSetAnimationImmediate( mshSkinnedModel, ANIM_RUN, 1, NULL, 0 );

	if ( strstr( szCommandLine, "-karate" ) )
	{
		// The kata is loaded at runtime (not linked into the .atm) as a spare animation use, and keeps its
		// root motion so the caveman moves around the floor. Camera follows the hips.
		msbKarateMode = TRUE;
		msbShowOriginal = FALSE;
		mxSkinnedModelPos.y = 0.0f;
		ModelSkeletonAddAnimation( mshSkinnedModel, KARATE_ANIMATION, ANIM_GESTURE_1, FALSE );
		ModelSkeletonSetRootMotion( mshSkinnedModel, TRUE );
		ModelSetAnimationImmediate( mshSkinnedModel, ANIM_GESTURE_1, 1, NULL, 0 );
		msnSceneMode = SCENE_MODE_VIDEO;
		FrameCaptureVideoStart( SKELOUT_FOLDER "\\caveman_karate_empi.mp4", InterfaceGetWidth(), InterfaceGetHeight(), VIDEO_FPS );
	}
	else if ( strstr( szCommandLine, "-video" ) )
	{
		msnSceneMode = SCENE_MODE_VIDEO;
		FrameCaptureVideoStart( SKELOUT_FOLDER "\\caveman_skeletal_run.mp4", InterfaceGetWidth(), InterfaceGetHeight(), VIDEO_FPS );
	}
	else if ( strstr( szCommandLine, "-shots" ) )
	{
		msnSceneMode = SCENE_MODE_STILLS;
	}
}

//-------------------------------------------------------------------------
// Dump shot mode : renders a fixed set of reference views of the original, saving each to a png
//-------------------------------------------------------------------------
static BOOL		SceneUpdateDumpShots( void )
{
int				nShot = msnFrameNum / 4;		// Render a few frames per shot so everything is settled
char			acFilename[256];
const char*		aszShotNames[] = { "front_negy", "side_posx", "back_posy", "side_negx" };
const float		afAngles[] = { -A90, 0.0f, A90, A180 };

	if ( nShot >= 4 ) return( FALSE );

	SceneSetOrbitCamera( afAngles[nShot], 4.0f, 0.9f, 0.9f );
	MainRenderFrame( 33 );
	if ( (msnFrameNum % 4) == 3 )
	{
		sprintf( acFilename, SKELOUT_FOLDER "\\shot_%s.png", aszShotNames[nShot] );
		FrameCaptureSaveScreenshot( acFilename );
	}
	InterfacePresent();
	msnFrameNum++;
	return( TRUE );
}

//-------------------------------------------------------------------------
// Video mode : fixed 30fps timestep, slow orbit, bone overlay in the second half
//-------------------------------------------------------------------------
static BOOL		SceneUpdateVideo( void )
{
uint32	ulTickIncrease = ( ((msnFrameNum + 1) * 1000) / VIDEO_FPS ) - ( (msnFrameNum * 1000) / VIDEO_FPS );
int		nNumFrames = msbKarateMode ? KARATE_VIDEO_NUM_FRAMES : VIDEO_NUM_FRAMES;
float	fPhase = (float)msnFrameNum / (float)nNumFrames;

	if ( msnFrameNum >= nNumFrames )
	{
		FrameCaptureVideoEnd();
		return( FALSE );
	}

	if ( msbKarateMode )
	{
	VECT	axJoints[MODEL_SKELETON_MAX_BONES];
	VECT	xPos;
	VECT	xTarget;
	float	fAngle = -0.5f - (fPhase * A360 * 0.75f);

		// Smoothly follow the hips (as of the last rendered frame) on the ground plane
		if ( ModelSkeletonGetCurrentJointPositions( mshSkinnedModel, axJoints, MODEL_SKELETON_MAX_BONES ) > 0 )
		{
		float	fFollow = ( msnFrameNum < 2 ) ? 1.0f : 0.06f;

			msxFollowTarget.x += ( (axJoints[0].x + mxSkinnedModelPos.x) - msxFollowTarget.x ) * fFollow;
			msxFollowTarget.y += ( (axJoints[0].y + mxSkinnedModelPos.y) - msxFollowTarget.y ) * fFollow;
		}
		xTarget = msxFollowTarget;
		xTarget.z = 0.8f;
		xPos.x = xTarget.x + (cosf( fAngle ) * 4.6f);
		xPos.y = xTarget.y + (sinf( fAngle ) * 4.6f);
		xPos.z = 1.5f;
		SceneSetCamera( &xPos, &xTarget );
		msbShowBones = FALSE;
	}
	else
	{
		msbShowBones = ( fPhase > 0.5f );
		SceneSetOrbitCamera( -0.35f - (fPhase * A360), 4.2f, 1.3f, 0.85f );
	}

	MainRenderFrame( ulTickIncrease );
	FrameCaptureVideoAddFrame();
	InterfacePresent();
	msnFrameNum++;
	return( TRUE );
}

//-------------------------------------------------------------------------
// Stills mode : a handful of frames from the run, from the side, to check the pose
//-------------------------------------------------------------------------
static BOOL		SceneUpdateStills( void )
{
char	acFilename[256];

	if ( msnFrameNum >= 48 ) return( FALSE );

	msbShowBones = TRUE;
	SceneSetOrbitCamera( ( msnFrameNum < 24 ) ? -A90 : -0.5f, 4.0f, 1.1f, 0.85f );
	MainRenderFrame( 1000 / VIDEO_FPS );
	if ( (msnFrameNum % 4) == 0 )
	{
		sprintf( acFilename, SKELOUT_FOLDER "\\still_%02d.png", msnFrameNum );
		FrameCaptureSaveScreenshot( acFilename );
	}
	InterfacePresent();
	msnFrameNum++;
	return( TRUE );
}

//-------------------------------------------------------------------------
BOOL		SceneUpdate( uint32 ulTickIncrease )
{
	switch( msnSceneMode )
	{
	case SCENE_MODE_DUMP_SHOTS:
		return( SceneUpdateDumpShots() );
	case SCENE_MODE_VIDEO:
		return( SceneUpdateVideo() );
	case SCENE_MODE_STILLS:
		return( SceneUpdateStills() );
	default:
		break;
	}

	msfCameraAngle -= ulTickIncrease * 0.0001f;
	SceneSetOrbitCamera( msfCameraAngle, 4.2f, 1.3f, 0.85f );
	MainRenderFrame( ulTickIncrease );
	InterfacePresent();
	return( TRUE );
}

//-------------------------------------------------------------------------
void		SceneRender( void )
{
VECT	xRot = { 0.0f, 0.0f, 0.0f };

	SceneApplyCamera();

	if ( msnSceneMode == SCENE_MODE_DUMP_SHOTS )
	{
	VECT	xPos = { 0.0f, 0.0f, 0.0f };

		ModelRender( mshOriginalModel, &xPos, &xRot, 0 );
		return;
	}

	if ( msbShowOriginal )
	{
		ModelRender( mshOriginalModel, &mxOriginalModelPos, &xRot, 0 );
	}
	ModelRender( mshSkinnedModel, &mxSkinnedModelPos, &xRot, 0 );
}

//-------------------------------------------------------------------------
// Function    : SceneDrawBones
// Description : Overlays the skinned model's skeleton as 2d lines (projected joint positions)
//-------------------------------------------------------------------------
static void		SceneDrawBones( int hModel, const VECT* pxModelPos )
{
VECT	axJoints[MODEL_SKELETON_MAX_BONES];
int		anScreenX[MODEL_SKELETON_MAX_BONES];
int		anScreenY[MODEL_SKELETON_MAX_BONES];
int		nNumBones = ModelSkeletonGetCurrentJointPositions( hModel, axJoints, MODEL_SKELETON_MAX_BONES );
int		nLoop;
const MODEL_SKELETON_BONE*	pxBone;

	for ( nLoop = 0; nLoop < nNumBones; nLoop++ )
	{
		VectAdd( &axJoints[nLoop], &axJoints[nLoop], pxModelPos );
		EngineGetScreenCoordForWorldCoord( &axJoints[nLoop], &anScreenX[nLoop], &anScreenY[nLoop] );
	}
	for ( nLoop = 0; nLoop < nNumBones; nLoop++ )
	{
		pxBone = ModelSkeletonGetBone( hModel, nLoop );
		if ( pxBone->nParentIndex != NOTFOUND )
		{
			InterfaceLine( 2, anScreenX[pxBone->nParentIndex], anScreenY[pxBone->nParentIndex], anScreenX[nLoop], anScreenY[nLoop], 0xFF40FF60, 0xFF40FF60 );
		}
		InterfaceRect( 2, anScreenX[nLoop] - 2, anScreenY[nLoop] - 2, 5, 5, 0xFFFFFF40 );
	}
}

//-------------------------------------------------------------------------
void		SceneDrawOverlay( void )
{
int		nOriginalX, nOriginalY;
int		nSkinnedX, nSkinnedY;
VECT	xLabelPos;

	if ( msnSceneMode == SCENE_MODE_DUMP_SHOTS ) return;

	if ( msbShowBones )
	{
		SceneDrawBones( mshSkinnedModel, &mxSkinnedModelPos );
	}

	if ( msbKarateMode )
	{
		InterfaceText( 3, 10, 10, "Antiquated Engine - skeletal animation experiment : caveman1.atm", 0xFFFFFFFF, 1 );
		InterfaceText( 3, 10, 40, "CMU mocap 135_02 : 'Empi' karate kata, retargeted onto the caveman's skeleton", 0xFFD0D0D0, 0 );
		return;
	}

	xLabelPos = mxSkinnedModelPos;
	xLabelPos.z = 1.95f;
	EngineGetScreenCoordForWorldCoord( &xLabelPos, &nSkinnedX, &nSkinnedY );
	InterfaceTextCentre( 3, nSkinnedX, nSkinnedY, "Skeletal (CMU mocap run, retargeted)", 0xFFFFFFFF, 0 );
	if ( msbShowOriginal )
	{
		xLabelPos = mxOriginalModelPos;
		xLabelPos.z = 1.95f;
		EngineGetScreenCoordForWorldCoord( &xLabelPos, &nOriginalX, &nOriginalY );
		InterfaceTextCentre( 3, nOriginalX, nOriginalY, "Original vertex-keyframe run", 0xFFD0D0D0, 0 );
	}
	InterfaceText( 3, 10, 10, "Antiquated Engine - skeletal animation experiment : caveman1.atm", 0xFFFFFFFF, 1 );
}

void		SceneOnKey( int nVirtKey )
{
	switch( nVirtKey )
	{
	case 'B': msbShowBones = !msbShowBones; break;
	case 'O': msbShowOriginal = !msbShowOriginal; break;
	}
}

void		SceneFree( void )
{
	ModelFree( mshSkinnedModel );
	ModelFree( mshOriginalModel );
}
