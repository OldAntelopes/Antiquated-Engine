#ifndef TRACTOR_RENDERING_H
#define TRACTOR_RENDERING_H

#include "Engine.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************
 ******  Model Rendering Stuff        *********
 **********************************************/

/**********************************************
 ******  Main Rendering system calls  *********
 **********************************************/

//	Main ModelRendering init, free, update
extern void		ModelRenderingFree( void );
extern void		ModelRenderingUpdate( uint32 );
extern void		ModelRenderingLateRenderPass( void );
extern void		ModelRenderingInit( void );
extern void		ModelRenderingFlush( void );

extern void		ModelRenderingSetShadowPass( BOOL bFlag );
extern BOOL		ModelRenderingIsShadowPass( void );
extern void		ModelRenderingActivateSceneShadowMap( BOOL bFlag );
extern BOOL		ModelRenderingIsSceneShadowMapActive( void );

// Handy resource management bit
extern void		ModelsFreeAll( void );


/**********************************************
 ******       Model Functions         *********
 **********************************************/
enum eRenderFlags    // For ulRenderFlags parameter in ModelRender
{
	RENDER_FLAGS_NO_EFFECTS = 0x1,
	RENDER_FLAGS_NO_FLUSH = 0x2,
	RENDER_FLAGS_LIGHTING_OVERRIDE = 0x4,
	RENDER_FLAGS_FORCE_SHADOWCAST = 0x10,
	RENDER_FLAGS_NO_SUBMODELS = 0x20,
	RENDER_FLAGS_NO_STATE_CHANGE = 0x80,
		RENDER_FLAGS_DRAWMESH_ONLY = (RENDER_FLAGS_NO_STATE_CHANGE|RENDER_FLAGS_LIGHTING_OVERRIDE|RENDER_FLAGS_NO_EFFECTS),
	RENDER_FLAGS_SHADOW_PASS = 0x100,
	RENDER_FLAGS_DONT_RECEIVE_SHADOWS = 0x200,
	RENDER_FLAGS_NO_FRONT_WHEELS = 0x400,
	RENDER_FLAGS_NO_REAR_WHEELS = 0x800,
	RENDER_FLAGS_MATERIAL1_ONLY = 0x1000,
	RENDER_FLAGS_MATERIAL2_ONLY = 0x2000,
	RENDER_FLAGS_MATERIAL3_ONLY = 0x4000,
	RENDER_FLAGS_MATERIAL4_ONLY = 0x8000,
	RENDER_FLAGS_WHEELS_ONLY = 0x10000,		// Draw only the attached wheels (no base mesh, turrets or effects). Pair with a NO_FRONT_WHEELS|NO_REAR_WHEELS render of the chassis


};

typedef	void		(*fnCustomMeshRenderer)( int nModelHandle, const VECT* pxPos, const VECT* pxRot, uint32 ulRenderFlags );



// Main load, render and free calls
extern int		ModelLoad( const char* szFilename, uint32 ulLoadFlags, float fScale ); // Returns a ModelHandle or NOTFOUND if unsuccessful
extern int		ModelLoadFromArchive( const char* szFilename, uint32 ulLoadFlags, float fScale, int nArchiveHandle ); // Returns a ModelHandle or NOTFOUND if unsuccessful
extern int		ModelCreateCustomRenderer( const char* szName, fnCustomMeshRenderer fnCustomRenderer, uint32 ulCreateParam );

extern int		ModelRender( int nModelHandle, const VECT* pxPos, const VECT* pxRot, uint32 ulRenderFlags );
extern int		ModelRenderSeparateWheel( int nModelHandle, int nWheelNum, const ENGINEMATRIX* pxWorldMat, uint32 ulRenderFlags );
extern int		ModelRenderQuat( int nModelHandle, const VECT* pxPos, const ENGINEQUATERNION* pxQuat, uint32 ulRenderFlags );
extern int		ModelRenderScaled( int nModelHandle, VECT* pxPos, VECT* pxRot, uint32 ulRenderFlags, VECT* pxScale );

extern void		ModelFree( int nModelHandle );
extern int		ModelClone( int nModelHandle );
extern int		ModelCopy( int nModelHandle );
extern void		ModelScale( int nModelHandle, float fX, float fY, float fZ );
extern void		ModelRotate( int nModelHandle, float fX, float fY, float fZ, BOOL bAffectSubModels );
extern void		ModelMoveVerts( int nModelHandle, float fX, float fY, float fZ );
extern int		ModelCreate( int nNumFaces, int nNumVerts, int nFlags );
extern int		ModelCreateCombinedModel( int nModelHandle );
extern void		ModelRecalcBounds( int nModelHandle );
extern void		ModelUpdateAnimationState(int nModelHandle);

//--------------------------------------------------------------------------------------------

typedef struct 
{

    VECT		position;	
    VECT		normal;		
    uint32		color;		
    float	    tu;		 
	float		tv;
		
	VECT		tangent;

} CUSTOMVERTEX;

typedef struct 
{

    VECT		position;	
    VECT		normal;		
    uint32		color;		
    float	    tu;		 
	float		tv;		 
    float	    tu2;		 
	float		tv2;		 
} CUSTOMVERTEXUV2;

typedef struct 
{

    VECT		position;	
    VECT		normal;		
    uint32		color;		
    float	    tu;		 
	float		tv;		 
    float	    tu2;		 
	float		tv2;		 
    float	    tu3;		 
	float		tv3;		 
} CUSTOMVERTEXUV3;

typedef struct 
{

    VECT		position;	
    VECT		normal;		
    uint32		color;		
    float	    tu;		 
	float		tv;		 
    float	    tu2;		 
	float		tv2;		 
    float	    tu3;		 
	float		tv3;		 
    float	    tu4;		 
	float		tv4;		 
} CUSTOMVERTEXUV4;

typedef struct
{
	int		nNumVertices;
	int		nNumNormals;
	int		nNumUVChannels;
	int		nNumColours;
	int		nNumIndices;
	int		nNumMaterials;
	int		nNumEmbeddedTextures;
	int		nFileSize;
	int		nNumVertKeyframes;
	int		nNumSubmodels;

	BOOL	boHasCollision;	

	VECT	xBoundBoxCentre;
	VECT	xBoundBoxExtents;
	VECT	xBoundMin;
	VECT	xBoundMax;
	VECT	xBoundSphereCentre;
	float	fBoundSphereRadius;

	char	acFilename[256];

	uint32	ulLockID;
	uint32	ulLockCode;

	BYTE	bNumLODs;
	BYTE	bLODUsed;
	BYTE	bHasWheels;
	BYTE	bHasHorizTurret;
	int		nNumCollisionMaps;

	char	acCreatorInfo[76];
	short	wPad1;
	short	wPad2;

} MODEL_STATS;


extern MODEL_STATS*		ModelGetStats( int nModelHandle );

//-----------------------------------------------------------------------------------------------

/**********************************************
 ******  Secondary Model Functions    *********
 **********************************************/

// Collision tests
extern BOOL		ModelRayTest( int nModelHandle, const VECT* pxPos, const VECT* pxRot, const VECT* pxRayStart, const VECT* pxRayEnd, VECT* pxHit, VECT* pxHitNormal, int* pnFaceNum, int flags );
extern BOOL		ModelSphereTest( int nModelHandle, const VECT* pxPos, const VECT* pxRot, const VECT* pxSpherePos, float fSphereRadius, VECT* pxHit, VECT* pxHitNormal );

// Secondary mesh access calls
extern VECT*	ModelGetVertexList( int nModelHandle, int* pnStride );
extern VECT*	ModelGetNormalList( int nModelHandle, int* pnStride );

extern void		ModelStoreVertexList( int nModelHandle );
extern void		ModelSetVertexColourAll( int nModelHandle, uint32 uARGB );

// Stuff for turrets, wheels etc..
extern VECT*	ModelGetAttachPoint( int nModelHandle, int nAttachNum );

extern BOOL		ModelDoesHaveHorizTurret( int nModelHandle );
extern BOOL		ModelDoesHaveVertTurret( int nModelHandle );
extern void		ModelSetHorizTurretRotation( int nModelHandle, float fHorizRot );
extern void		ModelSetVertTurretRotation( int nModelHandle, float fVertRot );
extern float	ModelGetHorizTurretRotation( int nModelHandle );
extern float	ModelGetVertTurretRotation( int nModelHandle );

extern void		ModelRenderSetWheelDisplayMode( int nModelHandle, int nMode, float fThrottle );
extern void		ModelSetWheelSpinRotation( int nModelHandle, float fVertRot );
extern void		ModelSetFrontWheelTurnRotation( int nModelHandle, float fRot );
// Wheel attach info, for games that move the wheels themselves (e.g. visual suspension).
// nWheelNum 0-3 = front pair (1,2) then rear pair (3,4). Attach point is in model space; radius is half the wheel model's height.
// Returns FALSE if the model has no wheel attached in that slot.
extern BOOL		ModelGetWheelAttachPoint( int nModelHandle, int nWheelNum, VECT* pxAttachPoint, float* pfWheelRadius );
extern int		ModelGetWheelDisplayMode( int nModelHandle );
// Model-space offset added to a wheel's attach point when it is drawn (NULL resets it). Also applied to the LOD models
extern void		ModelSetWheelRenderOffset( int nModelHandle, int nWheelNum, const VECT* pxOffset );

// Misc other bits
extern void		ModelSetMaterialEmissive( int nModelHandle, int nMaterialNum, const ENGINEMATERIAL_COLOUR* pxCol );
extern void		ModelSetVertexColours( int nModelHandle, uint32 ulCol );
extern void		ModelSetShadowCaster( int nModelHandle, int nMode );
extern int		ModelGetLockState( const char* szFilename );

extern void		RenderingComputeBoundingBox( const CUSTOMVERTEX* pVertices, int nNumVertices, VECT* pxBoundMin, VECT* pxBoundMax );
extern void		RenderingComputeBoundingSphere( const CUSTOMVERTEX* pVertices, int nNumVertices, VECT* pxSpherePos, float* pfSphereRadius );

//-------------------------------------------------------------------------------------------------------------------------------
// -----------------------------          Animation stuff

enum			// Used by ModelSetAnimationImmediate etc. (see below)
{				// (remember to update manAnimationMatchScores in ModelRendering.cpp when new usages are added here)
	ANIM_NONE,
	ANIM_WALK,
	ANIM_RUN,
	ANIM_DIE,
	ANIM_JUMP,
	ANIM_STANDING,	// 5
	ANIM_ENTER_BUILDING,
	ANIM_GENERAL_WEAPON_FIRE,
	ANIM_WEAPON_FIRE_PLASMA,
	ANIM_WEAPON_FIRE_MACHINEGUN,
	ANIM_WEAPON_FIRE_SPECIAL1, // 10
	ANIM_MOVE_STOP,
	ANIM_MOVE_START,
	ANIM_WAVE,
	ANIM_SMOKING,
	ANIM_STANDING_BORED,  // 15
	ANIM_STRAFE,
	ANIM_JETPACK,
	ANIM_RELOAD_GENERAL,
	ANIM_EAT,
	ANIM_DRINK,	// 20
	ANIM_DUCK,
	ANIM_DUCK_STAND_UP,
	ANIM_CRAWL,
	ANIM_LAY_PRONE,
	ANIM_DIVE_LEFT, // 25
	ANIM_DIVE_RIGHT,
	ANIM_FORWARD_ROLL,
	ANIM_BACKWARD_ROLL,
	ANIM_KICK,
	ANIM_GESTURE_1, // 30
	ANIM_GESTURE_2,
	ANIM_GESTURE_3,
	ANIM_DEAD,
	ANIM_SAT_DOWN,
	ANIM_COLLISION,
	ANIM_FIRE_WEAPON_MODEL_1,
	ANIM_FIRE_WEAPON_MODEL_2,
	ANIM_FIRE_WEAPON_MODEL_3,
	ANIM_FIRE_WEAPON_MODEL_4,
	ANIM_FIRE_WEAPON_MODEL_5,
	ANIM_FIRE_WEAPON_MODEL_6,
	ANIM_FIRE_WEAPON_MODEL_7,
	ANIM_FIRE_WEAPON_MODEL_8,
	ANIM_WALK_DAMAGED,
	ANIM_RUN_DAMAGED,
	ANIM_GENERAL_WEAPON_FIRE_WALKING,
	ANIM_GENERAL_WEAPON_FIRE_RUNNING,
	ANIM_GENERAL_WEAPON_FIRE_CRAWLING,
	ANIM_GENERAL_WEAPON_FIRE_PRONE,

	ANIM_BLEND_INTO_POSE1,
	ANIM_POSE1_BASE,				// 51
	ANIM_POSE1_IDLE1,
	ANIM_POSE1_IDLE2,
	ANIM_POSE1_IDLE3,
	ANIM_POSE1_IDLE4,
	ANIM_POSE1_ACTION1,
	ANIM_POSE1_ACTION2,
	ANIM_POSE1_ACTION3,
	ANIM_POSE1_ACTION4,
	ANIM_BLEND_OUT_OF_POSE1,

	ANIM_BLEND_INTO_POSE2,
	ANIM_POSE2_BASE,
	ANIM_POSE2_IDLE1,
	ANIM_POSE2_IDLE2,
	ANIM_POSE2_IDLE3,
	ANIM_POSE2_IDLE4,
	ANIM_POSE2_ACTION1,
	ANIM_POSE2_ACTION2,
	ANIM_POSE2_ACTION3,
	ANIM_POSE2_ACTION4,
	ANIM_BLEND_OUT_OF_POSE2,

	ANIM_BLEND_INTO_POSE3,
	ANIM_POSE3_BASE,
	ANIM_POSE3_IDLE1,
	ANIM_POSE3_IDLE2,
	ANIM_POSE3_IDLE3,
	ANIM_POSE3_IDLE4,
	ANIM_POSE3_ACTION1,
	ANIM_POSE3_ACTION2,
	ANIM_POSE3_ACTION3,
	ANIM_POSE3_ACTION4,
	ANIM_BLEND_OUT_OF_POSE3,

	ANIM_BLEND_INTO_POSE4,
	ANIM_POSE4_BASE,
	ANIM_POSE4_IDLE1,
	ANIM_POSE4_IDLE2,
	ANIM_POSE4_IDLE3,
	ANIM_POSE4_IDLE4,
	ANIM_POSE4_ACTION1,
	ANIM_POSE4_ACTION2,
	ANIM_POSE4_ACTION3,
	ANIM_POSE4_ACTION4,
	ANIM_BLEND_OUT_OF_POSE4,

	ANIM_RANDOM_IDLE_1,
	ANIM_RANDOM_IDLE_2,
	ANIM_RANDOM_IDLE_3,
	ANIM_RANDOM_IDLE_4,
	ANIM_RANDOM_IDLE_5,
	ANIM_RANDOM_IDLE_6,
	ANIM_RANDOM_IDLE_7,
	ANIM_RANDOM_IDLE_8,
	ANIM_RANDOM_IDLE_9,
	ANIM_RANDOM_IDLE_10,
	ANIM_RANDOM_IDLE_11,
	ANIM_RANDOM_IDLE_12,
	ANIM_RANDOM_IDLE_13,
	ANIM_RANDOM_IDLE_14,
	ANIM_RANDOM_IDLE_15,
	ANIM_RANDOM_IDLE_16,

	MAX_ANIMATION_USES // always at end
};

// Animation functions
extern int		ModelGetCurrentAnimation( int nModelHandle );

extern void		ModelSetAnimationImmediate( int nModelHandle, int nAnimationUse, int nPriority, void fnFunc(uint32), uint32 ulParam );
extern void		ModelSetAnimationNext( int nModelHandle, int nAnimationUse, int nPriority, void fnFunc(uint32), uint32 ulParam );
extern void		ModelSetAnimationSpeedModifier( int nModelHandle, float fAnimSpeedMod );

extern void		ModelAnimationPause( int nModelHandle, BOOL boFlag );
extern BOOL		ModelAnimationDoesHaveEventTrigger( int nModelHandle, int nAnim );
extern BOOL		ModelAnimationIsPresent( int nModelHandle, int nAnim );
extern BOOL		ModelIsAnimated( int nModelHandle );
extern BOOL		ModelShadowsEnabled( int nModelHandle );

extern int			ModelAnimationGetIDFromName( const char* szAnimName );
extern const char*	ModelAnimationGetName( int nAnimID );

//-------------------------------------------------------------------------------------------------------------------------------
// -----------------------------          Skeletal animation  (Experimental)
//
//  A model can carry a skeleton (bones with bind-pose joint positions) plus per-vertex bone weights, and
//  links to animation files (.bvh, in the same folder as the .atm). Animations from any humanoid source
//  skeleton are retargeted onto the model's bones by bone name at load time, then the mesh is skinned on
//  the CPU each frame. The usual ModelSetAnimationImmediate/Next calls select which linked animation plays.
//  See ATMFormat.md (chunks 0x1C-0x1E) and Pub\skeletalanimation.md.

#define		MODEL_SKELETON_MAX_BONES		128
#define		MODEL_SKELETON_BONE_NAME_LEN	32

typedef struct
{
	char	acName[MODEL_SKELETON_BONE_NAME_LEN];
	int		nParentIndex;			// NOTFOUND for the root. Parents must come before their children
	VECT	xBindPos;				// Joint position in model space, in the bind pose (the pose of the base mesh)

} MODEL_SKELETON_BONE;

typedef struct
{
	BYTE	abBoneIndex[4];
	float	afWeight[4];			// Sum to 1.0

} MODEL_SKIN_WEIGHT;

// The standard humanoid bone set (names follow the common Mixamo/CMU convention, minus any "mixamorig:" prefix)
enum eHumanoidBones
{
	HUMANOID_HIPS = 0,
	HUMANOID_SPINE,
	HUMANOID_SPINE1,
	HUMANOID_NECK,
	HUMANOID_HEAD,
	HUMANOID_LEFT_SHOULDER,
	HUMANOID_LEFT_ARM,
	HUMANOID_LEFT_FOREARM,
	HUMANOID_LEFT_HAND,
	HUMANOID_RIGHT_SHOULDER,
	HUMANOID_RIGHT_ARM,
	HUMANOID_RIGHT_FOREARM,
	HUMANOID_RIGHT_HAND,
	HUMANOID_LEFT_UPLEG,
	HUMANOID_LEFT_LEG,
	HUMANOID_LEFT_FOOT,
	HUMANOID_LEFT_TOEBASE,
	HUMANOID_RIGHT_UPLEG,
	HUMANOID_RIGHT_LEG,
	HUMANOID_RIGHT_FOOT,
	HUMANOID_RIGHT_TOEBASE,

	HUMANOID_NUM_BONES
};

extern const char*	ModelSkeletonGetHumanoidBoneName( int nHumanoidBone );

extern BOOL		ModelHasSkeleton( int nModelHandle );
extern BOOL		ModelSkeletonCreate( int nModelHandle, int nNumBones, const MODEL_SKELETON_BONE* pxBones );
// Creates the standard humanoid skeleton. pxJointPositions is indexed by eHumanoidBones (model space)
extern BOOL		ModelSkeletonCreateHumanoid( int nModelHandle, const VECT* pxJointPositions );
// Guesses joint positions for a humanoid mesh from its bounds (Z up, facing +X, arms down/A-pose)
extern void		ModelSkeletonEstimateHumanoidJoints( int nModelHandle, VECT* pxJointPositionsOut );
extern void		ModelSkeletonFree( int nModelHandle );

extern int		ModelSkeletonGetNumBones( int nModelHandle );
extern const MODEL_SKELETON_BONE*	ModelSkeletonGetBone( int nModelHandle, int nBoneNum );

extern void		ModelSkeletonSetVertexWeights( int nModelHandle, const MODEL_SKIN_WEIGHT* pxWeights );
extern const MODEL_SKIN_WEIGHT*	ModelSkeletonGetVertexWeights( int nModelHandle, int* pnNumWeightsOut );
extern void		ModelSkeletonAutoGenerateWeights( int nModelHandle, int nSmoothingPasses );

// Loads an animation (.bvh) and makes it available as nAnimUse (ANIM_RUN etc). Relative filenames are looked
// for in the model's folder. If bLinkInModel is set, the link is saved with the model by ModelExportATM.
// Returns the animation slot or NOTFOUND
extern int		ModelSkeletonAddAnimation( int nModelHandle, const char* szAnimFilename, int nAnimUse, BOOL bLinkInModel );
extern int		ModelSkeletonGetNumAnimations( int nModelHandle );
extern void		ModelSkeletonSetRootMotion( int nModelHandle, BOOL bKeepHorizontalRootMotion );

// Converts a vertex-keyframe model into a static mesh (in its current base pose) ready for skinning
extern void		ModelRemoveVertexAnimation( int nModelHandle );

// Current (last rendered) model-space joint positions, for debug drawing. Returns number of bones written
extern int		ModelSkeletonGetCurrentJointPositions( int nModelHandle, VECT* pxPositionsOut, int nMaxBones );


//-------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef DIRECT3D_VERSION		// only include this stuff if directx has been included already // Hacky bits :)
#if (DIRECT3D_VERSION>=0x0900)
#ifdef USE_D3DEX_INTERFACE
extern LPDIRECT3D9EX             g_pD3D       ;// Used to create the D3DDevice
extern LPDIRECT3DDEVICE9EX       g_pd3dDevice; // Our rendering device
#else
extern LPDIRECT3D9			     g_pD3D       ;// Used to create the D3DDevice
extern LPDIRECT3DDEVICE9		 g_pd3dDevice; // Our rendering device
#endif
#else
extern LPDIRECT3D8             g_pD3D       ;// Used to create the D3DDevice
extern LPDIRECT3DDEVICE8       g_pd3dDevice; // Our rendering device
#endif


#endif	// DIRECT3D_VERSION

CUSTOMVERTEX*		ModelLockVertexBuffer( int hModel );
void				ModelUnlockVertexBuffer( int hModel );
ushort*				ModelLockIndexBuffer( int hModel );
void				ModelUnlockIndexBuffer( int hModel );

extern BOOL			gboFog;		// wtf is this doin here then.


//-------------------------------------------------------------------------------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif
