
#ifndef MODEL_RENDERING_H
#define	MODEL_RENDERING_H

#include "EngineMesh.h"

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	

#define		MAX_MODELS_LOADED		1024		// Thats a feckin huge amount .... :}
#define		MAX_KEYFRAMES_IN_MODEL				255			// Cant be more than 255
#define		MAX_DIFFERENT_ANIMATIONS_IN_MODEL	32

enum
{
	MODEL_TYPE_STATIC_MESH = 0,
	MODEL_TYPE_KEYFRAME_ANIMATION,
	MODEL_TYPES_MAX,
};


enum
{
	MODEL_BLENDTYPES_NONE = 0,
	MODEL_BLENDTYPES_BLEND_ALPHA,
	MODEL_BLENDTYPES_ADDITIVE_ALPHA,
	MODEL_BLENDTYPES_SUBTRACTIVE_ALPHA,
	MODEL_BLENDTYPES_BLEND_COLOUR,
	MODEL_BLENDTYPES_ADDITIVE_COLOUR,
	MODEL_BLENDTYPES_SUBTRACTIVE_COLOUR,
	MODEL_BLENDTYPES_BLEND_ALPHA_NO_CUTOFF,
};

enum
{
	MODEL_ATTACHED_NONE = 0,
	MODEL_ATTACHED_TURRET_HORIZ,
	MODEL_ATTACHED_TURRET_VERT,
	MODEL_ATTACHED_WHEEL_1,
	MODEL_ATTACHED_WHEEL_2,
	MODEL_ATTACHED_WHEEL_3,
	MODEL_ATTACHED_WHEEL_4,
	MODEL_ATTACHED_LOD_2,
	MODEL_ATTACHED_LOD_3,
	MODEL_ATTACHED_CARRIED_1,
	MODEL_ATTACHED_CARRIED_2,
	MODEL_ATTACHED_COLLISION_MAP,
};

typedef struct
{
	ushort	uwKeyframeTime;
	BYTE	bAnimationUse;
	BYTE	bAnimationTriggerCode;
	
} MODEL_KEYFRAME_DATA;


typedef struct
{
	int				nCurrentAnimUse;
	int				nNextAnimUse;

	ushort			uwCurrentAnimPriority;
	ushort			uwNextAnimPriority;
		
	ulong			ulLastFrameTick;
	ulong			ulNextFrameTick;
	ushort			uwAnimNextFrame;
	ushort			uwAnimLastFrame;

	void	(*pfnCurrentTriggerFunc)( ulong ulParam );
	void	(*pfnNextTriggerFunc)( ulong ulParam );

	ulong		ulCurrentTriggerParam;
	ulong		ulNextTriggerParam;
	
	BOOL		boIsPaused;

} CURRENT_ANIMATION_STATE;

typedef struct
{
	BYTE	bBlendType;
	BYTE	bOpacity;
	BYTE	bBackfaceFlag;
	BYTE	bSpriteFlags;

	BYTE	bGlobalSpecularFlag;
	BYTE	bDontClampUVs;
	BYTE	bPad2;
	BYTE	bNoFiltering;

	float	fBounciness;
	BYTE	bCollisionOverride;
	BYTE	bLargeModelClipping;
	BYTE	bNoLighting;
	BYTE	bNoShadows;

} GLOBAL_PROPERTIES_CHUNK;


typedef struct		
{
	float	fHighDist;
	float	fMedDist;

} LOD_DATA;

typedef struct		
{
	int		nAttachVertex;
	VECT	xAttachOffset;

	int		nEffectType;
	ulong	ulEffectParam1;
	ulong	ulEffectParam2;

	ulong	ulLastEffectTick;
	ulong	ulLastEffectSwitch;

} EFFECT_ATTACH_DATA;

typedef struct		
{
	int		nModelHandle;
	int		nAttachVertex;

	VECT	xAttachOffset;

	VECT	xCurrentRotations;
	VECT	xCurrentOrigin;
	VECT	xRawOffset;

} TURRET_ATTACH_DATA;

typedef struct
{
	int		nAttachVertex;
	VECT	xAttachOffset;

	VECT	xTransformedPos;
	BOOL	boTransformValid;

} SINGLE_ITEM_ATTACH_DATA;

typedef struct		
{
	SINGLE_ITEM_ATTACH_DATA		xGenericWeaponFireAttach;
	SINGLE_ITEM_ATTACH_DATA		axWeaponFireAttach[8];
	
} LOCAL_ATTACH_DATA;

// The model render data structure keeps track of all the details of a model once it is loaded.
// This structure is not retained in any files so be changed without worrying about incompatabilities with older versions
typedef struct
{
	BYTE			bModelType;		// Stores the MODEL_TYPE enum described above
	BYTE			bAnimState;
	BYTE			bIsClone;		// Flag to indicate whether this model is a clone (If it is, ModelFree does not get rid of any keyframe buffers etc)
	BYTE			bLODOverride;

	EngineMesh*		pxBaseMesh;		// Stores the basic DX mesh for the current frame (which is currently used to render the model - this may be gotten rid of soon) 
	VECT*			pxVertexKeyframes;		// Stores the list of keyframes for a vertex-keyframe animation
	VECT*			pxNormalKeyframes;		// Stores the list of normal keyframes for a vertex-keyframe animation

	MODEL_STATS		xStats;				// Details about the model such as the number of vertices and normals etc.

	MODEL_KEYFRAME_DATA		axKeyframeData[MAX_KEYFRAMES_IN_MODEL];		// Information about each keyframe, such as the blend time and the animation it is used by

	CURRENT_ANIMATION_STATE		xAnimationState;	// Current active state of the animation, including which frame is currently being displayed, when the anim started etc etc
	
	TURRET_ATTACH_DATA		xHorizTurretData;
	TURRET_ATTACH_DATA		xVertTurretData;

	LOCAL_ATTACH_DATA		xAttachData;

	GLOBAL_PROPERTIES_CHUNK		xGlobalProperties;

	int			nCloneOfHandle;
	int			nCloneCount;

	VECT*	pxBaseVertices;		// Stores the list of vertex positions in the original (untransformed) basemesh
									// (Used to regenerate turret vertices etc..)
	TURRET_ATTACH_DATA		xWheel1AttachData;
	TURRET_ATTACH_DATA		xWheel2AttachData;
	TURRET_ATTACH_DATA		xWheel3AttachData;
	TURRET_ATTACH_DATA		xWheel4AttachData;
	TURRET_ATTACH_DATA		xMedLODAttachData;
	TURRET_ATTACH_DATA		xLowLODAttachData;
	TURRET_ATTACH_DATA		xCollisionAttachData;

	EFFECT_ATTACH_DATA		xEffectAttachData;

	BYTE		bShadowCastMode;
	BYTE		bWheelDisplayMode;
	BYTE		bHasEffect;
	BYTE		bPad4;
	ulong		ulLastShadowCastedTime;

	LOD_DATA		xLodData;

	class	BSPModel*		pBSPModel;
	class	ModelArchive*	pModelArchive;
	class	ModelMaterialData*		pMaterialData;
	class	SkinnedModel*	pSkinnedModel;

	float		fAnimSpeedMod;

} MODEL_RENDER_DATA;

//--------------------------------------------------------------------

extern int		ModelRenderGetNextHandle( void );

extern BOOL		ModelRenderDidCastShadow( void );

extern void		ModelRenderLODOverride( int nModelHandle, int nVal );

extern void		ModelRenderingAddEffect( MODEL_RENDER_DATA* pxModelData, VECT* pxPos );

extern void		ModelRenderingProcessStaticEffects( MODEL_RENDER_DATA* pxModelData, VECT* pxPos, int nUserParam );

extern void		ModelRenderKeyframeAnimationGenerateBaseMesh(  MODEL_RENDER_DATA* pxModelData, ulong ulTick );

extern void		ModelSetGlobalProperties( int nModelHandle, GLOBAL_PROPERTIES_CHUNK* pxProperties );

extern void		ModelRenderKeyframeAnimationUpdateBaseMeshFromKeyframes( MODEL_RENDER_DATA* pxModelData, VECT* pxVertKeyframes );
//-------------------------------------------------------------------


extern MODEL_RENDER_DATA		maxModelRenderData[ MAX_MODELS_LOADED ];

extern ulong	mulLastRenderingTick;


#ifdef __cplusplus
}
#endif


#endif
