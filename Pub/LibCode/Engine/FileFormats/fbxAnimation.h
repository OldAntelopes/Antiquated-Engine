#ifndef ENGINE_FBX_ANIMATION_H
#define ENGINE_FBX_ANIMATION_H

//-----------------------------------------------------------------------------------------------------------
// fbxAnimation
//
//  FBX <-> BVH skeletal animation conversion. Only available in the Tool build of the Engine (TOOL defined,
//  links the Autodesk FBX SDK) - used by the ModelConverter.
//
//  FbxAnimationConvertToBVH bakes the first animation stack of an FBX file's skeleton into a BVH whose rest
//  pose is the FBX bind (default) pose with identity rotations - which is what the Engine's skeleton
//  retargeting expects (see Pub\LibCode\Engine\Skeleton\ModelSkeleton.h). Scenes are converted to Y-up first.
//-----------------------------------------------------------------------------------------------------------

extern BOOL		FbxAnimationConvertToBVH( const char* szFBXFilename, const char* szBVHFilename, float fSampleFPS, char* szErrorOut, int nErrorLen );
extern BOOL		FbxAnimationCreateFromBVH( const char* szBVHFilename, const char* szFBXFilename, char* szErrorOut, int nErrorLen );

#endif
