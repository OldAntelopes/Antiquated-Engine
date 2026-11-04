#ifndef MODELCONV_SKELETAL_TOOLS_H
#define MODELCONV_SKELETAL_TOOLS_H

//-----------------------------------------------------------------------------------------------------------
// SkeletalTools
//
//  ModelConverter front-end for the (experimental) skeletal animation support :
//   - Import an FBX animation (e.g. from Mixamo) and save it as a .bvh the Engine can play
//   - Export a .bvh as FBX
//   - Rig the current model with the standard humanoid skeleton (estimated joints + automatic weights)
//   - Link a .bvh animation into the current model (saved in the .atm when you Save As)
//
//  Command line (no UI, exit code 0 on success) :
//		ModelConverter.exe -fbxanim2bvh <in.fbx> <out.bvh> [fps]
//		ModelConverter.exe -bvh2fbx <in.bvh> <out.fbx>
//-----------------------------------------------------------------------------------------------------------

extern BOOL		SkeletalToolsHandleCommandLine( const char* szCommandLine, int* pnExitCodeOut );

extern void		SkeletalToolsImportFBXAnimation( void );
extern void		SkeletalToolsExportBVHAsFBX( void );
extern void		SkeletalToolsAutoRigHumanoid( int hModel );
extern void		SkeletalToolsLinkAnimation( int hModel );

#endif
