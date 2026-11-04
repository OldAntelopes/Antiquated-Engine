#ifndef ENGINE_MODEL_SKELETON_H
#define ENGINE_MODEL_SKELETON_H

//-----------------------------------------------------------------------------------------------------------
// ModelSkeleton
//
//  The skeleton, skin weights and (retargeted) animations attached to a single model.
//  Owned by MODEL_RENDER_DATA::pSkeleton.
//
//  Retargeting (deliberately simple) :
//   For each model bone B mapped by name to a source joint J, with the source rest pose having identity
//   rotations (as BVH does) :
//     S_B(t) = Yaw * C * G_J(t) * C^-1 * A_B
//   where G_J(t) is J's global rotation in the clip, C converts the source axes (Y-up, facing +Z) to the
//   engine's (Z-up, facing +X), Yaw turns the clip's average heading to face +X, and A_B is the shortest
//   rotation taking B's bind-pose bone direction onto J's rest-pose bone direction. S_B maps the bind pose
//   to the animated pose, so joint positions come from FK on the model's own bone lengths and each vertex is
//     v' = sum_k w_k * ( S_k(t) * ( v - bind_k ) + joint_k(t) )
//-----------------------------------------------------------------------------------------------------------

#include "../ModelRendering.h"

class SkeletalAnimation;

#define		MODEL_SKELETON_MAX_ANIMS		16

typedef struct
{
	char				acFilename[64];		// As linked (relative to the model folder)
	int					nAnimUse;			// ANIM_RUN etc
	BOOL				bLinkInModel;		// Save the link when the model is exported
	SkeletalAnimation*	pSource;

	// Baked retargeted data
	int					nNumFrames;
	float				fFrameTime;
	ENGINEQUATERNION*	pxBoneRots;			// nNumFrames * numBones
	VECT*				pxRootPos;			// nNumFrames
	int					nLoopStart;
	int					nLoopEnd;			// exclusive

} MODEL_SKELETON_ANIM;


class ModelSkeleton
{
public:
	ModelSkeleton();
	~ModelSkeleton();

	void		Create( int nNumBones, const MODEL_SKELETON_BONE* pxBones );

	int							GetNumBones( void ) { return( mnNumBones ); }
	const MODEL_SKELETON_BONE*	GetBone( int nBone ) { return( &mpxBones[nBone] ); }
	int							FindBone( const char* szName );

	// Skin weights (one per vertex)
	void		SetWeights( int nNumVertices, const MODEL_SKIN_WEIGHT* pxWeights );
	const MODEL_SKIN_WEIGHT*	GetWeights( void ) { return( mpxWeights ); }
	int			GetNumWeights( void ) { return( mnNumWeights ); }
	void		AutoGenerateWeights( MODEL_RENDER_DATA* pxModelData, int nSmoothingPasses );
	void		RemapVertices( int nNumVertices, const DWORD* pulNewToOld );

	// Animations
	int			AddAnimation( const char* szLinkName, const char* szFullPath, int nAnimUse, BOOL bLinkInModel );
	int			GetNumAnimations( void ) { return( mnNumAnims ); }
	const MODEL_SKELETON_ANIM*	GetAnimation( int nAnim ) { return( &maxAnims[nAnim] ); }
	void		PlayAnimation( int nAnimUse, uint32 ulTick );
	void		SetKeepRootMotion( BOOL bFlag ) { mbKeepRootMotion = bFlag; }

	// Per-frame
	void		UpdateAndSkin( MODEL_RENDER_DATA* pxModelData, uint32 ulTick );
	void		RestoreBindPose( MODEL_RENDER_DATA* pxModelData );
	int			GetCurrentJointPositions( VECT* pxOut, int nMax );

private:
	void		CaptureBindMesh( MODEL_RENDER_DATA* pxModelData );
	int			GetDirectionChild( int nBone );
	void		BakeAnimation( MODEL_SKELETON_ANIM* pxAnim );
	void		FindLoop( MODEL_SKELETON_ANIM* pxAnim );
	void		SamplePose( const MODEL_SKELETON_ANIM* pxAnim, float fTime, ENGINEQUATERNION* pxRotsOut, VECT* pxRootPosOut );
	void		ComputeJointPositions( const ENGINEQUATERNION* pxRots, const VECT* pxRootPos, VECT* pxJointsOut );

	MODEL_SKELETON_BONE*	mpxBones;
	int						mnNumBones;

	MODEL_SKIN_WEIGHT*		mpxWeights;
	int						mnNumWeights;

	VECT*					mpxBindPositions;
	VECT*					mpxBindNormals;
	int						mnNumBindVertices;

	MODEL_SKELETON_ANIM		maxAnims[MODEL_SKELETON_MAX_ANIMS];
	int						mnNumAnims;
	int						mnCurrentAnim;
	uint32					mulAnimStartTick;
	BOOL					mbKeepRootMotion;

	ENGINEQUATERNION*		mpxCurrentRots;
	VECT*					mpxCurrentJoints;

	// Animate once per frame (opt-in, EngineSetAnimateOncePerFrame): what the last UpdateAndSkin wrote into which
	// mesh, so a repeat in the same frame (shadow map, reflection...) can reuse it. Clones share the skeleton and the
	// mesh, so they share the pose too.
	void					InvalidateSkinCache( void ) { mpLastSkinMesh = NULL; }
	const void*				mpLastSkinMesh;
	uint32					mulLastSkinTick;
	int						mnLastSkinAnim;
	uint32					mulLastSkinStartTick;
	float					mfLastSkinSpeed;
	BOOL					mbLastSkinKeepRootMotion;
};

// Loads an animation for a model - relative filenames are looked for in the model's folder
extern int		ModelSkeletonAddAnimationForModel( MODEL_RENDER_DATA* pxModelData, const char* szAnimFilename, int nAnimUse, BOOL bLinkInModel );

#endif
