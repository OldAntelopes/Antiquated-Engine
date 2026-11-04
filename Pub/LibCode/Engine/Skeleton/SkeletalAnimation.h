#ifndef ENGINE_SKELETAL_ANIMATION_H
#define ENGINE_SKELETAL_ANIMATION_H

//-----------------------------------------------------------------------------------------------------------
// SkeletalAnimation
//
//  A source animation clip in its own skeleton space, as loaded from a .bvh file.
//  (BVH is used as the interchange/animation file format for now - it's text, simple, and most mocap
//   libraries and DCC tools can export it. The ModelConverter can also write .bvh from FBX animations.)
//
//  Clips are not tied to any particular model - ModelSkeleton retargets them onto its own bones by
//  matching joint names (see SkeletalAnimationNormaliseBoneName for the matching rules).
//-----------------------------------------------------------------------------------------------------------

#define		SKELANIM_MAX_NAME_LEN		64

enum eBVHChannel
{
	BVH_CHANNEL_XPOS = 0,
	BVH_CHANNEL_YPOS,
	BVH_CHANNEL_ZPOS,
	BVH_CHANNEL_XROT,
	BVH_CHANNEL_YROT,
	BVH_CHANNEL_ZROT,
};

typedef struct
{
	char	acName[SKELANIM_MAX_NAME_LEN];
	int		nParentIndex;
	VECT	xOffset;				// Offset from parent joint, in the rest pose
	int		nChannelStart;			// Index of this joint's first channel in each frame's data
	int		nNumChannels;
	int		anChannelTypes[6];		// eBVHChannel, in file order
	BOOL	bHasEndSite;
	VECT	xEndSiteOffset;

} SKELANIM_JOINT;


class SkeletalAnimation
{
public:
	SkeletalAnimation();
	~SkeletalAnimation();

	BOOL		LoadBVH( const char* szFilename );
	BOOL		LoadBVHFromMem( const char* pcText, int nTextLen );

	int			GetNumJoints( void ) { return( mnNumJoints ); }
	const SKELANIM_JOINT*	GetJoint( int nJoint ) { return( &mpxJoints[nJoint] ); }
	int			FindJoint( const char* szName );

	int			GetNumFrames( void ) { return( mnNumFrames ); }
	float		GetFrameTime( void ) { return( mfFrameTime ); }

	// Global (skeleton space) rotation and position of every joint for the given frame
	void		GetGlobalPose( int nFrame, ENGINEQUATERNION* pxRotsOut, VECT* pxPositionsOut );
	// Global joint positions in the rest pose (all rotations zero)
	void		GetRestPositions( VECT* pxPositionsOut );

	const char*	GetFilename( void ) { return( macFilename ); }

	// Writes a BVH file. pfFrameData is nNumFrames * total channels (as described by the joint list).
	// Joints must be in depth-first (file) order, with nChannelStart matching that order.
	static BOOL	SaveBVH( const char* szFilename, const SKELANIM_JOINT* pxJoints, int nNumJoints, const float* pfFrameData, int nNumFrames, float fFrameTime );

private:
	void		Free( void );
	int			AddJoint( const char* szName, int nParent );

	char				macFilename[256];
	SKELANIM_JOINT*		mpxJoints;
	int					mnNumJoints;
	int					mnMaxJoints;
	int					mnTotalChannels;
	int					mnNumFrames;
	float				mfFrameTime;
	float*				mpfFrameData;
};

// Lower-cases and strips namespace prefixes ("mixamorig:LeftArm" -> "leftarm") so bones from different
// sources can be matched by name.
extern void		SkeletalAnimationNormaliseBoneName( const char* szIn, char* szOut, int nOutLen );

#endif
